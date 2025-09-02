#include "AlignModule.h"

AlignModule::AlignModule() = default;

AlignModule::~AlignModule() = default;

int AlignModule::Init(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) {
    std::cout << "AlignModule init start" << std::endl;
    this->AlignTraj = AlignTraj();
    return STATE_OK;
}

int AlignModule::Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) {
    frame



    static void align_se3(const std::vector<Eigen::Matrix<double, 7, 1>> &traj_es, const std::vector<Eigen::Matrix<double, 7, 1>> &traj_gt,
                        Eigen::Matrix3d &R, Eigen::Vector3d &t, int n_aligned = -1);

    return STATE_OK;
}

void AlignModule::PreCompute(std::string configPath) {
    return;
}

//需要检测时，通过RPC调用服务器上的检测功能
int AlignModule::CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr)
{
    double pose_timestamp = frameDataPtr->timestamp;
    std::string ret = get_tum_string(pose_timestamp, sys->getPose());


    bool detectionRequired = false;
    if (detectionRequired)
    {
        auto& frameData = *frameDataPtr;

        serilizedFrame.addRGBImage(frameData);  //添加RGB图像到serilizedFrame。serilizedFrame随后将被上传到服务器。
        // add depth
        serilizedFrame.addDepthImage(frameData);  // ? depth diff with RGB ???

        //编码过程调用的相关信息和数据
        SerilizedObjs send = {
            {"cmd",Bytes("detectPlane")},  //Bytes用于存贮对象序列化后的结果，对支持的类型可以通过Bytes构造函数直接序列化。对不支持的类型需要自己处理序列化和反序列化。
            
            {"pose",Bytes(ret)} //继续添加其它数据
        };

        procs.push_back(std::make_shared<RemoteProc>(this, frameDataPtr, send)); //添加到procs，随后该命令将被发送到ObjectTrackingServer进行处理
    }
    return STATE_OK;
}

int AlignModule::ProRemoteReturn(RemoteProcPtr proc){
    return STATE_OK;
}