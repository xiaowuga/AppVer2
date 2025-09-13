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

#include "RenderingGlass/pbrPass.h"
#include "RenderingGlass/equirectangularToCubemapPass.h"
#include "RenderingGlass/renderPassManager.h"
#include "RenderingGlass/irradiancePass.h"
#include "RenderingGlass/prefilterPass.h"
#include "RenderingGlass/brdfPass.h"
#include "RenderingGlass/backgroundPass.h"
#include "RenderingGlass/shadowMappingDepthPass.h"



namespace {


    std::shared_ptr<ARApp> construct_engine(){
        std::string appName="Relocation"; //APP名称，必须和服务器注册的App名称对应（由服务器上appDir中文件夹的名称确定）

        std::vector<ARModulePtr> modules;
        modules.push_back(createModule<ARInputs>("ARInputs"));
        modules.push_back(createModule<RelocationGlass>("Relocation"));  //用createModule创建模块，必须指定一个模块名，并且和server上的模块名对应！！
        modules.push_back(createModule<PoseEstimationFetch>("PoseEstimation"));  //用createModule创建模块，必须指定一个模块名，并且和server上的模块名对应！！
        modules.push_back(createModule<PoseEstimationRokid>("PoseEstimationRokid"));
//        modules.push_back(createModule<GestureUnderstanding>("GestureUnderstanding"));
//        modules.push_back(createModule<CollisionDetection>("CollisionDetection"));
//        modules.push_back(createModule<AnimationPlayer>("AnimationPlayer"));
        auto appData=std::make_shared<AppData>();
        auto sceneData=std::make_shared<SceneData>();

        appData->argc=1;
        appData->argv=nullptr;
        appData->engineDir="./AREngine/";  // for test
        appData->dataDir="./data/";        // for test

        //std::thread listenThread(listenForEvent, std::ref(*appData));

        std::shared_ptr<ARApp> app=std::make_shared<ARApp>();
        app->init(appName,appData,sceneData,modules);

        return app;
    }


    class SceneAppVer2 : public IScene{

    public:
        virtual bool initialize(const XrInstance instance,const XrSession session){
            _eng=construct_engine();
            mProject = glm::mat4(1.0);
            mView = glm::mat4(1.0);

            mModel  = std::make_shared<Model>("test");
            mModel->loadFbModel(MakeSdcardPath("RokidData/Model/YIBIAOPAN.fb"));
            mModel->loadModel("model/backpack/backpack.obj");
            auto& passManager = RenderPassManager::getInstance();
            // 初始化渲染通道、注册渲染通道
            mEquirectangularToCubemapPass = std::make_shared<EquirectangularToCubemapPass>();
            mEquirectangularToCubemapPass->initialize(mModel->getMMeshes());
            passManager.registerPass("equirectangularToCubemap", mEquirectangularToCubemapPass);

            mIrradiancePass = std::make_shared<IrradiancePass>();
            mIrradiancePass->initialize(mModel->getMMeshes());
            passManager.registerPass("irradiance", mIrradiancePass);

            mPrefilterPass = std::make_shared<PrefilterPass>();
            mPrefilterPass->initialize(mModel->getMMeshes());
            passManager.registerPass("prefilter", mPrefilterPass);

            mBrdfPass = std::make_shared<BrdfPass>();
            mBrdfPass->initialize(mModel->getMMeshes());
            passManager.registerPass("brdf", mBrdfPass);

            mShadowMappingDepthPass = std::make_shared<ShadowMappingDepthPass>();
            mShadowMappingDepthPass->initialize(mModel->getMMeshes());
            passManager.registerPass("shadowMappingDepth", mShadowMappingDepthPass);

            mPbrPass = std::make_shared<PbrPass>();
            mPbrPass->initialize(mModel->getMMeshes());
            passManager.registerPass("pbr", mPbrPass);

            mBackgroundPass = std::make_shared<BackgroundPass>();
            mBackgroundPass->initialize(mModel->getMMeshes());
            passManager.registerPass("background", mBackgroundPass);

            std::vector<std::string> passOrder = {"equirectangularToCubemap", "irradiance","prefilter","brdf","shadowMappingDepth","pbr","background"};
            passManager.setPassOrder(passOrder);

            _eng->connectServer("192.168.31.24",1299);
            _eng->start();

            return true;
        }
        virtual void renderFrame(const XrPosef &pose,const glm::mat4 &project,const glm::mat4 &view,int32_t eye){ //由于接口更改，以前的renderFrame函数不再适用，换用以下写法(2025-06-17)
            mProject=project; mView=view;
            if (_eng) {
                ARModulePtr modulePtr = _eng->getModule("Relocation");

                std::shared_ptr<RelocationGlass> relocationPtr = std::static_pointer_cast<RelocationGlass>(modulePtr);
                glm::mat4 model_trans_mat = glm::mat4(1.0);
                model_trans_mat = glm::inverse(relocationPtr->industrial_camera_world_pose) * relocationPtr->marker_industrial_camera_pose;
                mModel->render(mProject,mView,model_trans_mat);

                modulePtr = _eng->getModule("PoseEstimationRokid");
                std::shared_ptr<PoseEstimationRokid> poseEstimationRokidPtr = std::static_pointer_cast<PoseEstimationRokid>(modulePtr);
                poseEstimationRokidPtr->setHandJointLocation(this->m_app->getHandJointLocation());
                if(poseEstimationRokidPtr->hand_OK) {
                    std::vector<glm::mat4>& joc = poseEstimationRokidPtr->joint_loc;
                    mPbrPass->render(project, view, joc);
                }

            }
        }

        virtual void close(){
            if(_eng) _eng->stop();
        }



    public:
        std::shared_ptr<ARApp> _eng;

        std::shared_ptr<EquirectangularToCubemapPass> mEquirectangularToCubemapPass;
        std::shared_ptr<IrradiancePass> mIrradiancePass;
        std::shared_ptr<PrefilterPass> mPrefilterPass;
        std::shared_ptr<BrdfPass> mBrdfPass;
        std::shared_ptr<ShadowMappingDepthPass> mShadowMappingDepthPass;
        std::shared_ptr<PbrPass> mPbrPass;
        std::shared_ptr<BackgroundPass> mBackgroundPass;
        std::shared_ptr<Model> mModel;
        glm::mat4 mProject{},mView{};



    };

}


std::shared_ptr<IScene> _createScene_AppVer2(){
    return std::make_shared<SceneAppVer2>();
}