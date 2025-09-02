//
// Created by xiaow on 2025/8/28.
//

#include"arengine.h"
#include"scene.h"
#include "arucopp.h"
#include "utilsmym.hpp"
#include "markerdetector.hpp"
#include "model.h"
#include <glm/gtc/matrix_inverse.hpp>

static glm::mat4 ViewMat; //保存相机位姿，眼镜的Marker检测用
static glm::mat4 ArucoMat4=glm::mat4(1.0f); //调试用

glm::mat4 reloc_mat =glm::mat4(1.0f);

namespace {

    struct ArucoPose{
        std::vector<std::tuple<int,glm::mat4>> markers;
    };

    class Relocation : public ARModule {
    public:
        ArucoPP  _detector;
        std::vector<int> _retFrames;
        std::mutex _retFramesMutex;
        glm::mat4 ArucoPoseReturn{glm::mat4(1.0f)}; //保存接受到的pose，以便在不同的函数中使用
        glm::mat4 T_IM;
        glm::mat4 T_GM;

    public:
        int Init(AppData &appData,SceneData &sceneData,FrameDataPtr frameDataPtr){
            T_IM = glm::mat4(1.0);
            T_GM = glm::mat4(1.0);
            reloc_mat = glm::mat4(1.0);
            return STATE_OK;
        }

        int Update(AppData &appData,SceneData &sceneData,FrameDataPtr frameDataPtr){

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            cv::Mat sendMat,cameraMat; //要发送给服务器的Mat
            //====================== Aruco & Chessboard Detect ===============================
            if(!frameDataPtr->image.empty()){

                cv::Mat img=frameDataPtr->image.front(); //相机图像
                auto frame_data=std::any_cast<ARInputSources::FrameData>(sceneData.getData("ARInputs"));
                cv::Matx44f vmat = frame_data.cameraMat; //计算marker位姿需要的相机pose,这个也需要发送
                const cv::Matx33f &camK = frameDataPtr->colorCameraMatrix;

//                cameraMat=Matx44f_to_Mat(vmat);

                cv::Vec3d rvec, tvec;
                if (_detector.detect(img, camK, rvec, tvec)) {

                    T_GM =MarkerDetector::GetTransMatFromRT(rvec, tvec,
                                                              CV_Matx44f_to_GLM_Mat4(vmat));



                    reloc_mat = glm::inverse(CV_Mat_to_GLM_Mat4(vmat)) * T_GM * glm::inverse(T_IM);
                }


            }

            return STATE_OK;
        }

        int CollectRemoteProcs(SerilizedFrame &serilizedFrame,std::vector<RemoteProcPtr> &procs,FrameDataPtr frameDataPtr){
            auto &frameData=*frameDataPtr;
//            serilizedFrame.addRGBImage(frameData,0,".png"); //测试代码功能需要无损压缩，用png格式

            SerilizedObjs cmdSend = {
                    {"cmd", std::string("FetchIMMat")}
            };
//            app->postRemoteCall(this, nullptr, cmdSend);

            /*注意在CollectRemoteProcs中不要用App::postRemoteCall发送命令。
              因为视频帧数据是在所有模块的CollectRemoteProcs执行结束后才上传，因此在这里用App::postRemoteCall将导致命令比视频帧先送达服务器，这样命令将找不到对应的视频帧。
            */
            procs.push_back(std::make_shared<RemoteProc>(this,frameDataPtr,cmdSend,
                                                         RPCF_SKIP_BUFFERED_FRAMES)); //添加add命令，将输入帧的像素值加上指定的值，并返回结果图像(参考TestServer.cpp中TestPro1Server类的实现)

            return STATE_OK;
        }

        int ProRemoteReturn(RemoteProcPtr proc) {
            auto &send=proc->send;
            auto &ret=proc->ret;
            auto cmd=send.getd<std::string>("cmd");

            printf("ProRemoteReturn: Relocation cmd=%s, frameID=%d\n",cmd.c_str(),proc->frameDataPtr->frameID);

            if(cmd=="set"){//处理set命令返回值

                auto res =  ret.getd<std::vector<float>>("IMMat");
                memcpy(&T_IM, res.data(), sizeof(glm::mat4));
            }

            return STATE_OK;
        }

        int ShutDown(AppData &appData,SceneData &sceneData){
            return STATE_OK;
        }

        ArucoPose ArucoDetect(const cv::Mat &img,const cv::Matx44f &vmat){
            ArucoPose pose; //return value
            //detect and set pose
            cv::Mat cameraMatrix=RokidCameraMatrix, distCoeffs=RokidDistCoeffs;
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
        modules.push_back(createModule<Relocation>("Relocation"));  //用createModule创建模块，必须指定一个模块名，并且和server上的模块名对应！！

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


    class SceneAppVer2Init : public IScene{

        std::shared_ptr<ARApp> _eng;
        Model mModel{""};
        glm::mat4 mProject{},mView{};

    public:


        virtual bool initialize(const XrInstance instance,const XrSession session){
            _eng=construct_engine();
            mProject = glm::mat4(1.0);
            mView = glm::mat4(1.0);
            mModel.loadLocalModel("Download/TextureModel/Mars/mars.obj");

            _eng->connectServer("192.168.31.27",123);//connectServer("localhost", 123);
            _eng->start();
            return true;
        }
        virtual void renderFrame(const XrPosef &pose,const glm::mat4 &project,const glm::mat4 &view,int32_t eye){ //由于接口更改，以前的renderFrame函数不再适用，换用以下写法(2025-06-17)
            mProject=project; mView=view;
            if (_eng) {
                glm::mat4 model_trans_mat= reloc_mat * glm::mat4(0.0);
                mModel.render(project,view,model_trans_mat);
            }
        }

        virtual void close(){
            if(_eng) _eng->stop();
        }
    };

}


std::shared_ptr<IScene> _createScene_AppVer2_init(){
    return std::make_shared<SceneAppVer2Init>();
}