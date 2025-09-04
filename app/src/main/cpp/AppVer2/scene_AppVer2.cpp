//
// Created by xiaow on 2025/8/26.
//

#include "arengine.h"
#include "scene.h"
#include "PoseEstimationFetch.h"
#include "GestureUnderstanding.h"
#include "CollisionDetection.h"
#include "arucopp.h"
#include "utilsmym.hpp"
#include "markerdetector.hpp"
#include "demos/model.h"
#include "MyCollisionHandlers.h"
#include "json.hpp"
#include <filesystem>
#include "AnimationPlayer.h"

using namespace cv;

namespace
{

    std::shared_ptr<ARApp> construct_engine()
    {
        std::string appName = "TestApp"; // APP名称，必须和服务器注册的App名称对应（由服务器上appDir中文件夹的名称确定）

        std::vector<ARModulePtr> modules;
        modules.push_back(createModule<ARInputs>("ARInputs"));
        modules.push_back(createModule<PoseEstimationFetch>("PoseEstimation"));
        modules.push_back(createModule<GestureUnderstanding>("GestureUnderstanding"));
        modules.push_back(createModule<CollisionDetection>("CollisionDetection"));
        modules.push_back(createModule<AnimationPlayer>("AnimationPlayer")); // this module is used to play animations triggered by collisions
                                                                             //        modules.push_back(createModule<ObjectTracking2>("ObjectTracking2", &ObjectTracking2::create));  //用createModule创建模块，必须指定一个模块名，并且和server上的模块名对应！！

        auto appData = std::make_shared<AppData>();
        auto sceneData = std::make_shared<SceneData>();

        appData->argc = 1;
        appData->argv = nullptr;
        appData->engineDir = "./AREngine/"; // for test
        appData->dataDir = "./data/";       // for test
        appData->interactionConfigFile = "./InteractionConfig.json";
        appData->offlineDataDir = "";

        // we need to store this pointer in appData, we will use it when we want to set a new animator
        appData->setData("AnimationPlayer", modules[3]);

        std::shared_ptr<ARApp> app = std::make_shared<ARApp>();
        app->init(appName, appData, sceneData, modules);
        //        app->connectServer("192.168.31.27",123);
        //        std::any cmdData = std::string(MakeSdcardPath("Download/3d/box1/box1.3ds"));
        //        std::any cmdData = std::string(MakeSdcardPath("Download/3d/box1/box1.3ds"));

        return app;
    }

    class Scene_AppVer2 : public IScene
    {
    public:
        std::map<std::string, Model> mesh_list; // scene object name -> Model

        std::optional<Model> loadMeshFromFile(const std::string &model_name, const std::string &file_name)
        {
            bool res = true;
            auto local_path = MakeSdcardPath(file_name);
            Model model(model_name);
            model.initialize();
            if (!local_path.empty())
            {
                res = model.loadLocalModel(local_path);
                if (res)
                {
                    infof("Load Local Model Success: %s", local_path.c_str());
                    return model;
                }
                else
                {
                    errorf("Load Local Model Failed: %s", local_path.c_str());
                }
            }
            return std::nullopt;
        }
        virtual bool initialize(const XrInstance instance, const XrSession session)
        {
            _eng = construct_engine();
            _eng->connectServer("192.168.31.27", 123);

            auto coll_detection = _eng->getModule("CollisionDetection");
            coll_detection->Init(*_eng->appData, *_eng->sceneData, std::make_shared<FrameData>());

            std::list<std::string> model_files;
            try
            {
                for (const auto &entry : fs::directory_iterator(_eng->appData->dataDir + "Models"))
                {
                    if (entry.is_directory())
                    {
                        std::cout << "find model file: " << entry.path().filename().string() << std::endl;
                        model_files.push_back(entry.path().filename().string());
                    }
                }
            }
            catch (const fs::filesystem_error &e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
            }

            // structure
            // appData->[sceneObject]->{object_name, transform}
            // this->mesh_list: [<object_name, Model>]

            nlohmann::json instances_info_json;
            std::ifstream json_file(_eng->appData->dataDir + "InstanceInfo.json");
            json_file >> instances_info_json;
            // [ {"instanceId": string, "name": string, "matrixWorld": [float]}, {...}, ...]
            for (int i = 0; i < instances_info_json.size(); i++)
            {
                auto instance_info_json = instances_info_json[i];
                std::string object_name = instances_info_json.at("name").get<std::string>();
                if (find(model_files.begin(), model_files.end(), object_name) == model_files.end())
                {
                    // no corresponding mesh file to this instance, we will ignore it
                    continue;
                }
                std::vector<float> model_mat = instances_info_json.at("matrixWorld").get<std::vector<float>>();
                if (model_mat.size() != 16)
                {
                    std::cout << "number of elements in model matrix does not equal to 16!" << std::endl;
                }
                assert(model_mat.size() != 16);
                //                glm::mat4 model_mat_glm = glm::make_mat4(model_mat.data());
                cv::Matx44f model_mat_cv;
                std::copy(model_mat.begin(), model_mat.begin() + 16, model_mat_cv.val);

                std::string mesh_file_name = _eng->appData->dataDir + "Models/" + object_name + "/" + object_name + ".obj";
                _eng->sceneData->setObject(object_name, std::make_shared<SceneObject>(object_name, mesh_file_name, model_mat_cv));

                // load the corresponding mesh
                if (auto opt = loadMeshFromFile(object_name, mesh_file_name))
                {
                    mesh_list.insert(std::pair<std::string, Model>(object_name, *opt));
                }
            }

            _eng->start();
            return true;
        }

        virtual void inputEvent(int leftright, const ApplicationEvent &event)
        {
        }

        virtual void renderFrame(const XrPosef &pose, const glm::mat4 &project, const glm::mat4 &view, int32_t eye)
        {
            // 处理手势操作
        }

        virtual void processFrame()
        {
        }

        virtual void close()
        {
            if (_eng)
                _eng->stop();
        }

    private:
    public:
        std::shared_ptr<ARApp> _eng;
    };
}

std::shared_ptr<IScene> _createScene_Scene_AppVer2()
{
    return std::make_shared<Scene_AppVer2>();
}
