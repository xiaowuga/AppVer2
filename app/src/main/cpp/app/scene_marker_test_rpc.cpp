#include"Basic/include/RPC.h"
#include"Basic/include/ARModule.h"
#include"Basic/include/App.h"
#include"opencv2/core.hpp"
#include"arengine.h"
#include"scene.h"
#include"demos/utils.h"
#include"demos/model.h"
#include"utilsmym.hpp"

using namespace cv;
using namespace std;


static glm::mat4 ViewMat; //保存相机位姿，眼镜的Marker检测用
static glm::mat4 ArucoMat4=glm::mat4(1.0f); //调试用

//需要在服务器端开始运行后再运行眼镜端
namespace{
struct ArucoPose{
    std::vector<std::tuple<int,glm::mat4>> markers;
};

class TestPro1: public ARModule{
    std::vector<int> _retFrames;
    std::mutex _retFramesMutex;
    glm::mat4 ArucoPoseReturn{glm::mat4(1.0f)}; //保存接受到的pose，以便在不同的函数中使用
public:
    int Init(AppData &appData,SceneData &sceneData,FrameDataPtr frameDataPtr){
        return STATE_OK;
    }

    int Update(AppData &appData,SceneData &sceneData,FrameDataPtr frameDataPtr){
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        cv::Mat sendMat,cameraMat; //要发送给服务器的Mat
        //====================== Aruco & Chessboard Detect ===============================
        if(!frameDataPtr->image.empty()){
            cv::Mat img=frameDataPtr->image.front(); //相机图像
            auto frame_data=std::any_cast<ARInputSources::FrameData>(frameDataPtr->getData("ARInputs"));
            cv::Matx44f vmat=frame_data.cameraMat; //计算marker位姿需要的相机pose,这个也需要发送
            cameraMat=Matx44f_to_Mat(vmat);

            ArucoPose pose; //是从眼镜端直接检测到的marker pose,这里不使用，用从服务器发回来的new_pose
            pose=ArucoDetect(img,vmat); if(!pose.markers.empty()) ArucoMat4=std::get<1>(pose.markers[0]); //调试用

            auto trans=CalcCalibrationMat(img);
            if(!trans.empty()){ //计算完成，发送到服务器
                sendMat=trans;
            }
            //------------------ Test ----------------
            ArucoPose new_pose;
            new_pose.markers.emplace_back(2,ArucoPoseReturn);
            std::stringstream ss; ss<<"Send Mat Scene-Data: "<<GlmMat4_to_String(ArucoPoseReturn); infof(ss.str().c_str());
            sceneData.setData("ArucoPose",new_pose);
            //---------------------------------------
            //sceneData.setData("ArucoPose",pose);
        }
        //========================== Aruco Detect Done, Will Send Data ============================
        if(frameDataPtr->hasUploaded("RGB0")) //如果当前帧的RGB图像已经上传（通过CollectRemoteProcs)
            //如果没有上传又必须要进行远程调用，则需要通过send数据上传
            //这样不同模块可能重复上传同样的数据，因此应该尽量在CollectRemoteProcs中发送命令
        {
            std::stringstream ss; ss<<"Send Mat: "<<sendMat; infof(ss.str().c_str());
            SerilizedObjs cmdSend={
                    {"cmd",string("set")},
                    {"val",int(rand()%256)},
                    {"mat",sendMat}, //发送Mat
                    {"vmat",cameraMat}, //发送相机位姿
                    {"marker_pose",GLM_Mat4_to_CV_Mat(ArucoMat4)}, //发送计算好的Marker位姿，仅作调试，需要删除
            };
            app->postRemoteCall(this,frameDataPtr,cmdSend); //发送set命令，将图像的像素值设置为指定的值，并返回结果图像(参考TestServer.cpp中TestPro1Server类的实现)
        }

        while(false){
            {
                std::lock_guard<std::mutex> _lock(_retFramesMutex);
                if(std::find(_retFrames.begin(),_retFrames.end(),frameDataPtr->frameID)!=_retFrames.end())
                    break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return STATE_OK;
    }

    int CollectRemoteProcs(SerilizedFrame &serilizedFrame,std::vector<RemoteProcPtr> &procs,FrameDataPtr frameDataPtr){
        auto &frameData=*frameDataPtr;
        serilizedFrame.addRGBImage(frameData,0,".png"); //测试代码功能需要无损压缩，用png格式

        SerilizedObjs cmdSend={
                {"cmd",string("add")},
                {"val",int(rand())}
        };

        /*注意在CollectRemoteProcs中不要用App::postRemoteCall发送命令。
          因为视频帧数据是在所有模块的CollectRemoteProcs执行结束后才上传，因此在这里用App::postRemoteCall将导致命令比视频帧先送达服务器，这样命令将找不到对应的视频帧。
        */
        procs.push_back(std::make_shared<RemoteProc>(this,frameDataPtr,cmdSend,
                                                     RPCF_SKIP_BUFFERED_FRAMES)); //添加add命令，将输入帧的像素值加上指定的值，并返回结果图像(参考TestServer.cpp中TestPro1Server类的实现)

        return STATE_OK;
    }

    int ProRemoteReturn(RemoteProcPtr proc){
        auto &send=proc->send;
        auto &ret=proc->ret;
        auto cmd=send.getd<std::string>("cmd");

        printf("ProRemoteReturn: TestPro1 cmd=%s, frameID=%d\n",cmd.c_str(),proc->frameDataPtr->frameID);

        if(cmd=="set"){//处理set命令返回值
            Mat1b retImg=ret["result"].get<Image>();
            CV_Assert(retImg(0,0)==(uchar)send["val"].get<int>()); //验证像素值与val参数指定的值相等
            cv::Mat poseMat=ret["mat"].get<cv::Mat>(); //获取计算好的位姿
            if(!poseMat.empty()){
                ArucoPoseReturn=CV_Mat_to_GLM_Mat4(poseMat);
                std::stringstream ss; ss<<"Send Mat Return: "<<GlmMat4_to_String(ArucoPoseReturn); infof(ss.str().c_str());
            }
        }

        if(cmd=="add"){//处理add命令返回值
            Mat1b img=proc->frameDataPtr->image.front();
            Mat1b retImg=ret["result"].get<Image>();
//                CV_Assert(img(0, 0) == uchar(proc->frameDataPtr->frameID % 256));
            CV_Assert(retImg(0,0)==uchar(int(img(0,0)+send["val"].get<int>())%256)); //验证像素值为原像素值与val相加

            int fid=ret.getd<int>("curFrameID");
            std::lock_guard<std::mutex> _lock(_retFramesMutex);
            _retFrames.push_back(proc->frameDataPtr->frameID);
        }

        return STATE_OK;
    }
    int ShutDown(AppData &appData,SceneData &sceneData){
        return STATE_OK;
    }
    cv::Mat CalcCalibrationMat(const cv::Mat &cal_image){
        cv::Mat result;
        //detect and set pose
        cv::Mat cameraMatrix=RokidCameraMatrix,distCoeffs=RokidDistCoeffs;
        //************** Detect Marker ****************
        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners,rejectedCandidates;
        cv::aruco::DetectorParameters detectorParams=cv::aruco::DetectorParameters();
        cv::aruco::Dictionary dictionary=cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_50);
        cv::aruco::ArucoDetector detector(dictionary,detectorParams);
        detector.detectMarkers(cal_image,markerCorners,markerIds,rejectedCandidates);
        if(!markerCorners.empty()){
            //if(markerIds.size()>1) warnf("Detected More than 1 Markers.");
            for(int i=0;i<(int)markerCorners.size();++i){
                if(markerIds[i]!=25) continue;
                // 估计相机姿态
                static cv::Vec3d rvec,tvec;
                float markerLength=140*0.001f;  // 每个标记的边长（单位：米） 例如 150mm
                float marker_offset=markerLength/2.0f;
                std::vector<cv::Point3f> objectPoints={ // 定义标记的 3D 坐标（四个角点的坐标）
                        cv::Point3f(0-marker_offset,0-marker_offset,0),            // 第一个角点 (左下)
                        cv::Point3f(markerLength-marker_offset,0-marker_offset,0), // 第二个角点 (右下)
                        cv::Point3f(markerLength-marker_offset,markerLength-marker_offset,0), // 第三个角点 (右上)
                        cv::Point3f(0-marker_offset,markerLength-marker_offset,0)  // 第四个角点 (左上)
                };
                bool pnp_flag=cv::fisheye::solvePnP(objectPoints,markerCorners[i],cameraMatrix,distCoeffs,rvec,tvec,false);
                if(!pnp_flag) errorf("ArucoDetect PnP Fail!"); //PnP Fail!
                std::stringstream ss;
                ss<<"PnP: Rotation Vector: "<<rvec<<", Translation Vector: "<<tvec;
                infof(ss.str().c_str());

                result=Matx44f_to_Mat(FromRT(rvec, tvec, false));
            }
        }
        return result;
    }
    ArucoPose ArucoDetect(const cv::Mat &img,const cv::Matx44f &vmat){
        ArucoPose pose; //return value
        //detect and set pose
        cv::Mat cameraMatrix=RokidCameraMatrix,distCoeffs=RokidDistCoeffs;
        //************** Detect Marker ****************
        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners,rejectedCandidates;
        cv::aruco::DetectorParameters detectorParams=cv::aruco::DetectorParameters();
        cv::aruco::Dictionary dictionary=cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_50);
        cv::aruco::ArucoDetector detector(dictionary,detectorParams);
        detector.detectMarkers(img,markerCorners,markerIds,rejectedCandidates);
        if(!markerCorners.empty()){
            cv::Mat outputImage=img.clone();
            cv::Mat colorImage;
            cv::cvtColor(outputImage,colorImage,cv::COLOR_GRAY2BGR);
            //if(markerIds.size()>1) warnf("Detected More than 1 Markers.");
            for(int i=0;i<(int)markerCorners.size();++i){
                if(markerIds[i]!=2) continue;
                // 估计相机姿态
                static cv::Vec3d rvec,tvec;
                float markerLength=7.9*0.01f;  // 每个标记的边长（单位：米） 例如 74.95mm
                float marker_offset=markerLength/2.0f;
                std::vector<cv::Point3f> objectPoints={ // 定义标记的 3D 坐标（四个角点的坐标）
                        cv::Point3f(0-marker_offset,0-marker_offset,0),            // 第一个角点 (左下)
                        cv::Point3f(markerLength-marker_offset,0-marker_offset,0), // 第二个角点 (右下)
                        cv::Point3f(markerLength-marker_offset,markerLength-marker_offset,0), // 第三个角点 (右上)
                        cv::Point3f(0-marker_offset,markerLength-marker_offset,0)  // 第四个角点 (左上)
                };
                bool pnp_flag=cv::fisheye::solvePnP(objectPoints,markerCorners[i],cameraMatrix,distCoeffs,rvec,tvec,false);
                if(!pnp_flag) errorf("ArucoDetect PnP Fail!"); //PnP Fail!
                std::stringstream ss;
                ss<<"PnP: Rotation Vector: "<<rvec<<", Translation Vector: "<<tvec;
                infof(ss.str().c_str());


                auto tmp1=FromRT(cv::Vec3f(0,0,0),cv::Vec3f(0,0,0),true);
                auto tmp_false=(tmp1*FromRT(rvec,tvec,false)).t();
                ViewMat=CV_Matx44f_to_GLM_Mat4(vmat);
                glm::mat4 trans=glm::inverse(ViewMat)*CV_Matx44f_to_GLM_Mat4(tmp_false);


                pose.markers.emplace_back(markerIds[i],trans);
            }
        }
        return pose;
    }
};

std::shared_ptr<ARApp> construct_engine(){
    std::string appName="TestApp"; //APP名称，必须和服务器注册的App名称对应（由服务器上appDir中文件夹的名称确定）

    std::vector<ARModulePtr> modules;
    modules.push_back(createModule<ARInputs>("ARInputs"));
    modules.push_back(createModule<TestPro1>("TestPro1"));  //用createModule创建模块，必须指定一个模块名，并且和server上的模块名对应！！

    auto appData=std::make_shared<AppData>();
    auto sceneData=std::make_shared<SceneData>();

    appData->argc=1;
    appData->argv=nullptr;
    appData->engineDir="./AREngine/";  // for test
    appData->dataDir="./data/";        // for test

    //std::thread listenThread(listenForEvent, std::ref(*appData));

    std::shared_ptr<ARApp> app=std::make_shared<ARApp>();
    app->init(appName,appData,sceneData,modules);

    app->connectServer("192.168.31.27",123);
    // app.connectServer("10.102.32.173", 123);
    //app.run();
    return app;
}


class SceneMarkerTestRPC: public IScene{
    std::shared_ptr<ARApp> _eng;
    int _poseGrabId=-1;
    typedef std::tuple<Model,glm::mat4> ModelItemType; //该cpp中应该不需要用到第二项，translate mat从api获取
    std::vector<ModelItemType> modelList; //<Model, ModelTransMat>
    float mDefaultScale=1.0f;//0.011f;
public:
    bool add_model_from_file(const std::string &model_name,const std::string &file_name){
        bool res=true;
        auto local_path=MakeSdcardPath(file_name);
        Model model(model_name);
        model.initialize();
        if(!local_path.empty()){
            res=model.loadLocalModel(local_path);
            if(res){
//            modelList.emplace_back(model,glm::mat4(1.0f));
                infof("Load Local Model Success: %s",local_path.c_str());
            }
            else errorf("Load Local Model Failed: %s",local_path.c_str());
            modelList.emplace_back(model,glm::mat4(1.0f));
        }
        else res=false;
        return res;
    }

    virtual bool initialize(const XrInstance instance,const XrSession session){
         _eng=construct_engine();
        _eng->connectServer("192.168.31.27",123);//connectServer("localhost", 123);
//        _poseGrabId=_eng->addGrabFunctorT([](ARApp *app){ //2025-06-13 此段代码无法通过编译，发现其在scene_engine_test_aruco.app中被注释了，因此这里也注释掉
//            ArucoPose pose;
//            std::any data=app->sceneData->getData("ArucoPose");
//            if(data.has_value())
//                pose=std::any_cast<ArucoPose>(data);
//            return pose;
//        });
        add_model_from_file("MyModel2","Download/TextureModel/Mars2/mars.obj"); //这里用了两个模型，一个用眼睛直接检测的结果，一个用服务器返回的结果，对比调试用
        add_model_from_file("MyModel","Download/TextureModel/Mars/mars.obj");

        _eng->start();
        return true;
    }
    virtual void renderFrame(const XrPosef &pose,const glm::mat4 &project,const glm::mat4 &view,int32_t eye){ //由于接口更改，以前的renderFrame函数不再适用，换用以下写法(2025-06-17)
        if (_eng) {
            auto _res=_eng->sceneData->getData("ArucoPose");
            if(_res.has_value()){
                auto res = std::any_cast<ArucoPose>(_res);
                for(const auto &marker:res.markers){
                    auto [marker_id,marker_pose]=marker;
                    //也可以判断一下marker id,看看是不是设置的marker
                    for(int i=1;i<(int)modelList.size();++i){ //正常应该从0开始，这里第一个模型用作对比
                        auto model=std::get<0>(modelList[i]);
                        glm::mat4 model_trans_mat=marker_pose;//std::get<1>(modelList[i]);
                        infof(std::string("Model Trans Mat: "+GlmMat4_to_String(model_trans_mat)).c_str());
                        model.render(project,view,model_trans_mat);
                    }
                    std::get<0>(modelList[0]).render(project,view,ArucoMat4); //调试用，显示眼镜直接检测到的marker，用作对比
                }
            }
        }
    }
//    virtual void renderFrame(const XrPosef &pose,const glm::mat4 &project,const glm::mat4 &view,int32_t eye){
//        if(_eng){
//            auto [hasMarker,markerPose]=_eng->getGrabbedT<ArucoPose>(_poseGrabId);
//            //render virtual object with markerPose
////            infof(std::string("Marker Pose Size: "+std::to_string(markerPose.markers.size())).c_str());
//            if(hasMarker){
//                for(const auto &marker:markerPose.markers){
//                    auto [marker_id,marker_pose]=marker;
//                    //也可以判断一下marker id,看看是不是设置的marker
//                    for(int i=1;i<(int)modelList.size();++i){ //正常应该从0开始，这里第一个模型用作对比
//                        auto model=std::get<0>(modelList[i]);
//                        glm::mat4 model_trans_mat=marker_pose;//std::get<1>(modelList[i]);
//                        infof(std::string("Model Trans Mat: "+GlmMat4_to_String(model_trans_mat)).c_str());
//                        model.render(project,view,model_trans_mat);
//                    }
////
//                }
//            }
//        }
//        std::get<0>(modelList[0]).render(project,view,ArucoMat4); //调试用，显示眼镜直接检测到的marker，用作对比
//    }

    virtual void close(){
        if(_eng) _eng->stop();
    }
};

}


std::shared_ptr<IScene> _createScene_marker_test_rpc(){
    return std::make_shared<SceneMarkerTestRPC>();
}