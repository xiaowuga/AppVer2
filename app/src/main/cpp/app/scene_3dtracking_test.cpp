#include"ObjectTracking2/include/ObjectTracking2.h"
#include"Basic/include/App.h"
#include<opencv2/highgui.hpp>
#include<opencv2/calib3d.hpp>
#include"BFC/portable.h"
#include"arengine.h"
#include"scene.h"
#include"demos/utils.h"
#include"demos/model.h"
#include"utilsmym.hpp"
using namespace cv;

static glm::mat4 ViewMat; //保存相机位姿，眼镜的Marker检测用

namespace {

    struct ArucoPose {
        std::vector<std::tuple<int, cv::Matx33f, cv::Vec3f>> markers;
    };

    class ArucoDetector: public ARModule {
        //std::vector<int> _retFrames;
        //std::mutex _retFramesMutex;
    public:
        int Init(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) {
            return STATE_OK;
        }
        int Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) {
            if (!frameDataPtr->image.empty()) {
                cv::Mat img = frameDataPtr->image.front();
                auto frame_data=std::any_cast<ARInputSources::FrameData>(frameDataPtr->getData("ARInputs"));
                cv::Matx44f vmat=frame_data.cameraMat;

                cv::Mat cameraMatrix = (cv::Mat)frameDataPtr->colorCameraMatrix;
                cv::Mat distCoeffs;// = GetDistCoeffs(0.002581,0.168011,-0.265207,0.246430,-0.075304);
                std::vector<int> markerIds;
                std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
                cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
                cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_50);
                cv::aruco::ArucoDetector detector(dictionary, detectorParams);
                detector.detectMarkers(img, markerCorners, markerIds, rejectedCandidates);

                ObjectTracking2::Result result;
                result.camK=cameraMatrix;

                if(!markerCorners.empty()) {
                    cv::Mat outputImage = img.clone();
                    cv::Mat colorImage;
                    cv::cvtColor(outputImage, colorImage, cv::COLOR_GRAY2BGR);

                    if (markerIds.size() > 1) warnf("Detected More than 1 Markers.");
                    //if (markerIds[0] != 2);//return;
                    // 估计相机姿态
                    cv::Vec3d rvec, tvec;
                    float markerLength = 7.9 * 0.01f;  // 每个标记的边长（单位：米） 例如 74.95mm
                    float marker_offset = markerLength / 2.0f;
                    std::vector<cv::Point3f> objectPoints = { // 定义标记的 3D 坐标（四个角点的坐标）
                            cv::Point3f(0 - marker_offset, 0 - marker_offset,0),            // 第一个角点 (左下)
                            cv::Point3f(markerLength - marker_offset, 0 - marker_offset,0), // 第二个角点 (右下)
                            cv::Point3f(markerLength - marker_offset, markerLength - marker_offset,0), // 第三个角点 (右上)
                            cv::Point3f(0 - marker_offset, markerLength - marker_offset,0)  // 第四个角点 (左上)
                    };
                    bool pnp_flag = cv::solvePnP(objectPoints, markerCorners[0], cameraMatrix,distCoeffs, rvec, tvec, false);
                    if (pnp_flag) { //PnP Fail!
                        cv::Matx33f R;
                        cv::Rodrigues(rvec, R);

                        ObjectTracking2::Result::ModelPose pose;
                        pose.modelIndex=markerIds[0];
                        pose.R=R;
                        pose.t=tvec;
                        pose.modelName="aruco";
                        result.objPoses.push_back(pose);
                    }
                }

                sceneData.setData("ObjectTracking2Result", result);
            }

            return STATE_OK;
        }
    };

    std::shared_ptr<ARApp> construct_engine() {
        std::string appName = "TestApp"; //APP名称，必须和服务器注册的App名称对应（由服务器上appDir中文件夹的名称确定）

        std::vector<ARModulePtr> modules;
        modules.push_back(createModule<ARInputs>("ARInputs"));
        modules.push_back(createModule<ObjectTracking2>("ObjectTracking2", &ObjectTracking2::create));  //用createModule创建模块，必须指定一个模块名，并且和server上的模块名对应！！

        auto appData = std::make_shared<AppData>();
        auto sceneData = std::make_shared<SceneData>();

        appData->argc = 1;
        appData->argv = nullptr;
        appData->engineDir = "./AREngine/";  // for test
        appData->dataDir = "./data/";        // for test

        std::shared_ptr<ARApp> app = std::make_shared<ARApp>();
        app->init(appName, appData, sceneData, modules);

        std::any cmdData = std::string(MakeSdcardPath("Download/3d/box1/box1.3ds"));
        app->call("ObjectTracking2", ObjectTracking2::CMD_SET_MODEL, cmdData);

        return app;
    }


    class Scene_3dtracking_test:public IScene {
        std::shared_ptr<ARApp> _eng;
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
                if(model.loadLocalModel(local_path)){
                    infof("Load Local Model Success: %s",local_path.c_str());
                }
                else errorf("Load Local Model Failed: %s",local_path.c_str());
                modelList.emplace_back(model, glm::mat4(1.0f));
            } else res = false;
            return res;
        }
        virtual bool initialize(const XrInstance instance, const XrSession session) {
            _eng = construct_engine();
            add_model_from_file("MyModel","Download/TextureModel/Mars/mars.obj");
            _eng->start();
            return true;
        }

        virtual void renderFrame(const XrPosef &pose, const glm::mat4 &project, const glm::mat4 &view,int32_t eye) {

            if (_eng) {
                auto _res=_eng->sceneData->getData("ObjectTracking2Result");
                auto _inputs=_eng->sceneData->getData("ARInputs");
                glm::mat4 vmat;
                if(_inputs.has_value()){
                    vmat=CV_Matx44f_to_GLM_Mat4(std::any_cast<ARInputSources::FrameData>(_inputs).cameraMat);
                    infof(("Get ViewMat: "+GlmMat4_to_String(vmat)).c_str())
                }
                if(_res.has_value()){
                    auto res = std::any_cast<ObjectTracking2::Result>(_res);

                    for(const auto &pose:res.objPoses){
                        cv::Matx44f trans=FromR33T(pose.R,pose.t*0.001,false);

                        auto tmp1=FromRT(cv::Vec3f(0,0,0),cv::Vec3f(0,0,0),true);
                        auto tmp_false=(tmp1*trans).t();
                        glm::mat4 model_trans_mat=glm::inverse(vmat)*CV_Matx44f_to_GLM_Mat4(tmp_false);

                        infof(("Model trans Mat: "+GlmMat4_to_String(model_trans_mat)).c_str());
                        auto model=std::get<0>(modelList[0]);
                        model.render(project,view,model_trans_mat);
                    }

//                    //也可以判断一下marker id,看看是不是设置的marker
//                    for(int i=1;i<(int)modelList.size();++i){ //正常应该从0开始，这里第一个模型用作对比
//                        auto model=std::get<0>(modelList[i]);
//                        glm::mat4 model_trans_mat=marker_pose;//std::get<1>(modelList[i]);
//                        infof(std::string("Model Trans Mat: "+GlmMat4_to_String(model_trans_mat)).c_str());
//                        model.render(project,view,model_trans_mat);
//                    }
                }
            }
        }

        virtual void close() {
            if (_eng) _eng->stop();
        }
    };
}

std::shared_ptr<IScene> _createScene_3dtracking_test() {
    return std::make_shared<Scene_3dtracking_test>();
}


