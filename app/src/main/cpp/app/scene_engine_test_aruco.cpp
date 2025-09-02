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

static glm::mat4 ViewMat;

glm::mat4 test_model_trans_mat;
bool ENABLE_STATIC_MODEL = false;

namespace {
    struct ArucoPose {
//        std::vector<std::tuple<int, cv::Matx34f>> markers;
        std::vector<std::tuple<int, glm::mat4>> markers;
    };

    class ArucoDetector: public ARModule {
        std::vector<int> _retFrames;
        std::mutex _retFramesMutex;
    public:
        int Init(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) {
            return STATE_OK;
        }
        int Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) {
            if (!frameDataPtr->image.empty()) {
                cv::Mat img = frameDataPtr->image.front();
                auto frame_data=std::any_cast<ARInputSources::FrameData>(sceneData.getData("ARInputs"));
                cv::Matx44f vmat=frame_data.cameraMat;
                ArucoPose pose;
                //detect and set pose
                //************** Detect Marker ****************
                cv::Mat cameraMatrix = GetCameraMatrix(290.4418,290.5276,316.4334,241.5392);
                cv::Mat distCoeffs = GetDistCoeffs(0.002581,0.168011,-0.265207,0.246430,-0.075304);
                std::vector<int> markerIds;
                std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
                cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
                cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_50);
                cv::aruco::ArucoDetector detector(dictionary, detectorParams);
                detector.detectMarkers(img, markerCorners, markerIds, rejectedCandidates);
                if(!markerCorners.empty()) {
                    cv::Mat outputImage = img.clone();
                    cv::Mat colorImage;
                    cv::cvtColor(outputImage, colorImage, cv::COLOR_GRAY2BGR);
//                    for(int i=0;i<(int)markerIds.size();++i){  //Print Marker Info
//                        std::string marker_info("Get Marker Id: "+std::to_string(markerIds[i])+" At TimeStamp: "+std::to_string(timestamp)+". Pos:");
//                        for(auto j:markerCorners[i]) marker_info+=std::string(" ("+std::to_string(j.x)+", "+std::to_string(j.y)+")");
//                        infof(marker_info.c_str());
//                    }
                    if (markerIds.size() > 1) warnf("Detected More than 1 Markers.");
                    //if (markerIds[0] != 2);//return;
                    // 估计相机姿态
                    static cv::Vec3d rvec, tvec;
                    float markerLength = 7.9 * 0.01f;  // 每个标记的边长（单位：米） 例如 74.95mm
                    float marker_offset = markerLength / 2.0f;
                    std::vector<cv::Point3f> objectPoints = { // 定义标记的 3D 坐标（四个角点的坐标）
                            cv::Point3f(0 - marker_offset, 0 - marker_offset,0),            // 第一个角点 (左下)
                            cv::Point3f(markerLength - marker_offset, 0 - marker_offset,0), // 第二个角点 (右下)
                            cv::Point3f(markerLength - marker_offset, markerLength - marker_offset,0), // 第三个角点 (右上)
                            cv::Point3f(0 - marker_offset, markerLength - marker_offset,0)  // 第四个角点 (左上)
                    };
                    bool pnp_flag = cv::solvePnP(objectPoints, markerCorners[0], cameraMatrix,distCoeffs, rvec, tvec, false);
                    if (!pnp_flag) { //PnP Fail!
                    }
                    std::stringstream ss;
                    ss << "PnP: Rotation Vector: " << rvec << ", Translation Vector: " << tvec;
                    infof(ss.str().c_str());
                    auto tmp = FromRT(cv::Vec3f(0, 0, 0), cv::Vec3f(0, 0, 0), true) *FromRT(rvec, tvec, true);
                    //tmp=tmp.inv();
                    infof(("tmp mat4: " + GlmMat4_to_String(CV_Matx44f_to_GLM_Mat4(tmp)) +"\n").c_str());
                    auto glm_tmp = glm::mat4(0.997157, 0.0104888, -0.0746256, 0,
                                             -0.031623, 0.9571, -0.288027, 0,
                                             0.0684032, 0.289568, 0.95471, 0,
                                             0.0518217, -0.468377, 0.0681542, 1);
                    ViewMat=CV_Matx44f_to_GLM_Mat4(vmat);
                    auto trans = glm::inverse(ViewMat) * CV_Matx44f_to_GLM_Mat4(tmp);

                    ss.clear();
                    ss << "trans glmmat4: " << GlmMat4_to_String(CV_Matx44f_to_GLM_Mat4(tmp))<< "\n---------------\n" << GlmMat4_to_String(ViewMat) << std::endl;
                    infof(ss.str().c_str());

                    pose.markers.push_back(std::make_tuple(markerIds[0],trans));
                }


                sceneData.setData("ArucoPose", pose);
            }

            return STATE_OK;
        }
    };

    std::shared_ptr<ARApp> construct_engine() {
        std::string appName = "AppVer2"; //APP名称，必须和服务器注册的App名称对应（由服务器上appDir中文件夹的名称确定）

        std::vector<ARModulePtr> modules;
        modules.push_back(createModule<ARInputs>("ARInputs"));
        modules.push_back(createModule<ArucoDetector>("ArucoDetector"));  //用createModule创建模块，必须指定一个模块名，并且和server上的模块名对应！！


        auto appData = std::make_shared<AppData>();
        auto sceneData = std::make_shared<SceneData>();

        appData->argc = 1;
        appData->argv = nullptr;
        appData->engineDir = "./AREngine/";  // for test
        appData->dataDir = "./data/";        // for test

        //std::thread listenThread(listenForEvent, std::ref(*appData));

        std::shared_ptr<ARApp> app = std::make_shared<ARApp>();
        app->init(appName, appData, sceneData, modules);

        app->connectServer("192.168.31.27", 123);//
        // app.connectServer("10.102.32.173", 123);
        //app.run();
        return app;
    }


    class Scene_engine_test_aruco: public IScene {
        std::shared_ptr<ARApp> _eng;
       // int _poseGrabId = -1;
        typedef std::tuple<Model, glm::mat4> ModelItemType; //该cpp中应该不需要用到第二项，translate mat从api获取
        std::vector<ModelItemType> modelList; //<Model, ModelTransMat>
        float mDefaultScale = 1.0f;//0.011f;
    public:
        bool add_model_from_file(const std::string &model_name, const std::string &file_name) {
            bool res = true;
            auto local_path = MakeSdcardPath(file_name);
            Model model(model_name);
            model.initialize();
            if (!local_path.empty()) {
                res = model.loadLocalModel(local_path);
                if (res) {
//            modelList.emplace_back(model,glm::mat4(1.0f));
                    infof("Load Local Model Success: %s",local_path.c_str());
                }
                else errorf("Load Local Model Failed: %s",local_path.c_str());
                modelList.emplace_back(model, glm::mat4(1.0f));
            } else res = false;
            return res;
        }
        virtual bool initialize(const XrInstance instance, const XrSession session) {
            _eng = construct_engine();

//            _poseGrabId = _eng->addGrabFunctorT([](ARApp *app) {
//                ArucoPose pose;
//                std::any data = app->sceneData->getData("ArucoPose");
//                if (data.has_value())
//                    pose = std::any_cast<ArucoPose>(data);
//                return pose;
//            });
            add_model_from_file("MyModel", "Download/TextureModel/Mars/mars.obj");
            _eng->start();
            return true;
        }

        virtual void renderFrame(const XrPosef &pose, const glm::mat4 &project, const glm::mat4 &view,int32_t eye) {
            if(ENABLE_STATIC_MODEL){
                for(int i=0;i<(int)modelList.size();++i) {
                    auto model = std::get<0>(modelList[i]);
                    model.render(project, view, test_model_trans_mat);//std::get<1>(modelList[i]));
                }
            }else{
                if (_eng) {
                    auto result=_eng->sceneData->getData("ArucoPose");

                    //render virtual object with markerPose
                    if(result.has_value()){
                        auto markerPose=std::any_cast<ArucoPose>(result);

                        for(const auto &marker:markerPose.markers){
                            auto [marker_id,marker_pose]=marker;
                            //也可以判断一下marker id,看看是不是设置的marker
                            for(int i=0;i<(int)modelList.size();++i) {
                                auto model = std::get<0>(modelList[i]);
                                auto model_trans_mat = marker_pose;//std::get<1>(modelList[i]);
//                            glm::mat4 scale_model = glm::mat4(1.0f);
//                            scale_model = glm::scale(model_trans_mat,glm::vec3(mDefaultScale, mDefaultScale,mDefaultScale));

                                model.render(project, view, model_trans_mat);//std::get<1>(modelList[i]));
                                test_model_trans_mat = model_trans_mat;
//                                ENABLE_STATIC_MODEL = true;
                            }
                        }
                    }
                }
            }

        }

        virtual void close() {
            if (_eng)
                _eng->stop();
        }
    };

}


std::shared_ptr<IScene> _createScene_engine_test_aruco() {
    return std::make_shared<Scene_engine_test_aruco>();
}
