
#include "ObjectTracking.h"
#include <QApplication>
#include <QThread>
#include"RbotTracking.h"

ObjectTracking::ObjectTracking() = default;

ObjectTracking::~ObjectTracking() = default;


int ObjectTracking::Init(AppData & appData, SceneData & sceneData, FrameDataPtr frameDataPtr) {

    if (!appData.hasData("QApplication"))
    {
        auto qapp=std::make_shared<QApplication>(appData.argc, appData.argv);
        appData.setData("QApplication", qapp);
    }

    tracker = new RbotTracking();
    // appData.engineDir refer to config.json
    std::string trackingConfigPath = "/ObjectTracking/config.json";
    std::string cameraConfigPath = "/App/CameraConfigs/";
    std::string engineDir = appData.engineDir;
    
    ConfigLoader crdr(engineDir+trackingConfigPath);
    std::string cameraConfigName = crdr.getValue<std::string>("cameraConfig");
    tracker->data_path = crdr.getValue<std::string>("dataPath"); //load data
    std::string cameraConfigFolderName = "CameraConfigs/";
    cv::Matx44f initPose = crdr.get44Matrix("initPose");
    bool loop = crdr.getValue<bool>("loopMode");
    std::string modelName = crdr.getValue<std::string>("modelName");
    std::string modelObjName = crdr.getValue<std::string>("modelObjName");
     
    ConfigLoader camera_crdr = ConfigLoader(engineDir+cameraConfigPath+cameraConfigName);

    appData.params.intrinsicMatrices.push_back(camera_crdr.getCamIntrinsicMatrix());
    appData.params.distCoeffs.push_back(camera_crdr.getCamDistCoeff());
    appData.params.shaderPath = engineDir + crdr.getValue<std::string>("relativeShaderPath");
    
    // //scene data construction
    // ObjectPose objectPose=ObjectPose();
    // objectPose.pose.setPose(initPose);
    
    // objectPose.modelName=modelName;
    // objectPose.modelPath=tracker->data_path + modelObjName;
    // sceneData.objectPose.push_back(objectPose);

    if(loop){
        std::string suffix = crdr.getValue<std::string>("imageSuffix");
        frameDataPtr->image.push_back(cv::imread(tracker->data_path+suffix));
    }
    tracker->Init(appData, sceneData, frameDataPtr);
    std::cout << "ObjectTracking Init successed" << std::endl;
    return STATE_OK;
}

int ObjectTracking::Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) {
    // frameDataPtr->image.push_back(cv::imread(this->tracker->data_path + "frame.png"));
    std::cout << "ObjectTracking Update ..." << std::endl;

    tracker->Update(appData, sceneData, frameDataPtr);
    std::cout << "ObjectTracking Update successed" << std::endl;
    return STATE_OK;
}

void ObjectTracking::PreCompute(std::string configPath) {
    return;
}

int ObjectTracking::ShutDown(AppData& appData, SceneData& sceneData){
    // 实现关闭逻辑
    std::cout << "ObjectTracking module shutting down." << std::endl;
    return STATE_OK;
}

//需要检测时，通过RPC调用服务器上的检测功能
int ObjectTracking::CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr)
{
    bool detectionRequired = false;

    if (detectionRequired)
    {
        auto& frameData = *frameDataPtr;

        serilizedFrame.addRGBImage(frameData);  //添加RGB图像到serilizedFrame。serilizedFrame随后将被上传到服务器。

        //编码过程调用的相关信息和数据
        SerilizedObjs send = {
            {"cmd",Bytes("detect")},  //Bytes用于存贮对象序列化后的结果，对支持的类型可以通过Bytes构造函数直接序列化。对不支持的类型需要自己处理序列化和反序列化。
            
            {"initPose",Bytes(cv::Matx34f::eye())} //继续添加其它数据
        };

        procs.push_back(std::make_shared<RemoteProc>(this, frameDataPtr, send)); //添加到procs，随后该命令将被发送到ObjectTrackingServer进行处理
    }
    return STATE_OK;
}

#include"ObjectTrackingServer.h"

int ObjectTracking::ProRemoteReturn(RemoteProcPtr proc)
{
    auto cmd = proc->send["cmd"].get<std::string>();
    if (cmd == "detect")
    {
        auto detectedObjs = proc->ret["results"].getv<ObjectTrackingServer::DetectedObj>();

        //应用检测结果，注意处理线程同步问题
    }

    return STATE_OK;
}

