//
// Created by xiaow on 2025/8/28.
//

//#include"arengine.h"
#include"scene.h"
//#include "arucopp.h"
#include "utilsmym.hpp"
#include "markerdetector.hpp"
#include "demos/model.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <common/xr_linear.h>

#include "ARInput.h"
#include "RelacationGlass.h"
#include "PoseEstimationFetch.h"
#include "PoseEstimationRokid.h"
#include "PoseEstimationFetch.h"


#include "GestureUnderstanding.h"
#include "CollisionDetection.h"
#include "AnimationPlayer.h"
#include "MyCollisionHandlers.h"

#include "RenderClient.h"


namespace {


    std::shared_ptr<ARApp> construct_engine(){
        std::string appName="Relocation"; //APP名称，必须和服务器注册的App名称对应（由服务器上appDir中文件夹的名称确定）

        std::vector<ARModulePtr> modules;
        modules.push_back(createModule<ARInputs>("ARInputs"));
//        modules.push_back(createModule<RelocationGlass>("Relocation"));  //用createModule创建模块，必须指定一个模块名，并且和server上的模块名对应！！
//        modules.push_back(createModule<PoseEstimationFetch>("PoseEstimation"));  //用createModule创建模块，必须指定一个模块名，并且和server上的模块名对应！！
        modules.push_back(createModule<PoseEstimationRokid>("PoseEstimationRokid"));
        modules.push_back(createModule<GestureUnderstanding>("GestureUnderstanding"));
        modules.push_back(createModule<CollisionDetection>("CollisionDetection"));
        modules.push_back(createModule<AnimationPlayer>("AnimationPlayer"));
        auto ptr = std::static_pointer_cast<AnimationPlayer>(modules.back());
        auto appData=std::make_shared<AppData>();
        auto sceneData=std::make_shared<SceneData>();

        appData->argc=1;
        appData->argv=nullptr;
        appData->engineDir="./AREngine/";  // for test
        appData->dataDir="/storage/emulated/0/AppVer2Data/";        // for test
        appData->interactionConfigFile = "InteractionConfig.json";
        appData->offlineDataDir = "";

        // we need to store this pointer in appData, we will use it when we want to set a new animator
        appData->setData("AnimationPlayer", ptr);

        std::shared_ptr<ARApp> app=std::make_shared<ARApp>();
        app->init(appName,appData,sceneData,modules);

        return app;
    }


    class SceneAppVer2 : public IScene{

    public:
        virtual bool initialize(const XrInstance instance,const XrSession session){

            _eng=construct_engine();

            std::string dataDir = _eng->appData->dataDir;
            Rendering = createModule<RenderClient>("RenderClient");

            auto frameData=std::make_shared<FrameData>();
            Rendering->Init(*_eng->appData.get(), *_eng->sceneData.get(), frameData);

//            _eng->connectServer("192.168.31.24",1299);
            _eng->start();

            return true;
        }
        virtual void renderFrame(const XrPosef &pose,const glm::mat4 &project,const glm::mat4 &view,int32_t eye){ //由于接口更改，以前的renderFrame函数不再适用，换用以下写法(2025-06-17)
            auto frameData=std::make_shared<FrameData>();
            Rendering->project = project;
            Rendering->view = view;
            if (_eng) {
                std::shared_ptr<RelocationGlass> relocationPtr = std::static_pointer_cast<RelocationGlass>(_eng->getModule("Relocation"));
                glm::mat4 model_trans_mat = glm::mat4(1.0);
                if(relocationPtr) {
                    model_trans_mat = glm::inverse(relocationPtr->industrial_camera_world_pose) * relocationPtr->marker_industrial_camera_pose;
                }

                std::shared_ptr<PoseEstimationRokid> poseEstimationRokidPtr = std::static_pointer_cast<PoseEstimationRokid>(_eng->getModule("PoseEstimationRokid"));
                if(poseEstimationRokidPtr != nullptr) {
                    std::vector<glm::mat4> &joc = poseEstimationRokidPtr->get_joint_loc();
                    Rendering->joc = joc;
                }

            }
            Rendering->Update(*_eng->appData.get(), *_eng->sceneData.get(), frameData);
        }

        virtual void close(){
            if(_eng) _eng->stop();
        }



    public:
        std::shared_ptr<ARApp> _eng;
        std::shared_ptr<RenderClient> Rendering = createModule<RenderClient>("RenderClient");



    };

}


std::shared_ptr<IScene> _createScene_AppVer2(){
    return std::make_shared<SceneAppVer2>();
}