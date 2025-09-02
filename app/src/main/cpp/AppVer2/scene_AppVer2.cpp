//
// Created by xiaow on 2025/8/26.
//

#include"arengine.h"
#include"scene.h"
#include "PoseEstimationFetch.h"
#include "GestureUnderstanding.h"
#include "CollisionDetection.h"
#include "arucopp.h"
#include "utilsmym.hpp"
#include "markerdetector.hpp"

using namespace cv;

extern glm::mat4 reloc_mat;


namespace {


    std::shared_ptr<ARApp> construct_engine() {
        std::string appName = "TestApp"; //APP名称，必须和服务器注册的App名称对应（由服务器上appDir中文件夹的名称确定）

        std::vector<ARModulePtr> modules;
        modules.push_back(createModule<ARInputs>("ARInputs"));
        modules.push_back(createModule<PoseEstimationFetch>("PoseEstimation"));
        modules.push_back(createModule<GestureUnderstanding>("GestureUnderstanding"));
        modules.push_back(createModule<CollisionDetection>("CollisionDetection"));
//        modules.push_back(createModule<ObjectTracking2>("ObjectTracking2", &ObjectTracking2::create));  //用createModule创建模块，必须指定一个模块名，并且和server上的模块名对应！！

        auto appData = std::make_shared<AppData>();
        auto sceneData = std::make_shared<SceneData>();

        appData->argc = 1;
        appData->argv = nullptr;
        appData->engineDir = "./AREngine/";  // for test
        appData->dataDir = "./data/";        // for test

        std::shared_ptr<ARApp> app = std::make_shared<ARApp>();
        app->init(appName, appData, sceneData, modules);
//        app->connectServer("192.168.31.27",123);
//        std::any cmdData = std::string(MakeSdcardPath("Download/3d/box1/box1.3ds"));
//        std::any cmdData = std::string(MakeSdcardPath("Download/3d/box1/box1.3ds"));

        return app;
    }

    class Scene_AppVer2 : public IScene {
    public:
        virtual bool initialize(const XrInstance instance,const XrSession session) {
            _eng=construct_engine();
            _eng->connectServer("192.168.31.27",123);

            _eng->start();
            return true;
        }

        virtual void inputEvent(int leftright, const ApplicationEvent& event) {

        }

        virtual void renderFrame(const XrPosef &pose,const glm::mat4 &project,const glm::mat4 &view,int32_t eye){
            if (_eng) {

            }
        }

        virtual void processFrame() {

        }

        virtual void close(){
            if(_eng) _eng->stop();
        }

    private:


    public:
        std::shared_ptr<ARApp> _eng;
    };
}

std::shared_ptr<IScene> _createScene_Scene_AppVer2() {
    return std::make_shared<Scene_AppVer2>();
}
