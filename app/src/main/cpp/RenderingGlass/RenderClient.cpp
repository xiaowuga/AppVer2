//
// Created by shutiao on 2025/9/13.
//

#include "RenderClient.h"
#include <android/log.h>
#include <fstream>
#include <jni.h>
#include"opencv2/core.hpp"
#include "app/utilsmym.hpp"
#include <opencv2/opencv.hpp>

#define ACTION = false;
#define LOG_TAG "RenderClient.cpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define HAND_JOINT_COUNT 16

RenderClient::RenderClient() = default;

RenderClient::~RenderClient() = default;

//TODO：现在没有传进来的关节位姿，后续需要添加

int RenderClient::Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) {
    LOGI("RenderClient init");


    mModel  = std::make_shared<renderModel>("test");
    mModel->shaderInit();
//    sceneData.getObject<SceneModel>();
//    mModel->loadFbModel("YIBIAOPAN.fb" ,appData.dataDir + "Models");
//    mModel->loadFbModel(appData.dataDir + "Models/YIBIAOPAN.fb");
//    mModel->loadFbModel("YIBIAOPAN.fb","/storage/emulated/0/Download/FbModel");
    mModel->loadFbModel("TUILIGAN.fb","/storage/emulated/0/Download/FbModel");
//    mModel->loadFbModel("ranyoukongzhiqi.fb","/storage/emulated/0/Download/FbModel");

//    auto scene_virtualObjects = sceneData.getAllObjectsOfType<SceneModel>();
//    for(int i = 0; i < scene_virtualObjects.size(); i ++){
//        std::string name = scene_virtualObjects[i]->name;
//        std::string filet_path = scene_virtualObjects[i]->filePath;
//        mModel->loadFbModel(name, filet_path);
//    }

    mModel->pushMeshFromCustomData();

    //加载模型的动画数据：Action + State
    cadDataManager::DataInterface::loadAnimationActionData(appData.animationActionConfigFile);
    cadDataManager::DataInterface::loadAnimationStateData(appData.animationStateConfigFile);

//    mModel->loadModel("model/backpack/backpack.obj");
    // 获取RenderPassManager单例
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

    mSSAOGeometryPass = std::make_shared<SSAOGeometryPass>();
    mSSAOGeometryPass->initialize(mModel->getMMeshes());
    passManager.registerPass("SSAOGeometry", mSSAOGeometryPass);

    mSSAOPass = std::make_shared<SSAOPass>();
    mSSAOPass->initialize(mModel->getMMeshes());
    passManager.registerPass("SSAO", mSSAOPass);


    // 设置渲染顺序（先环境贴图转换，后PBR渲染）
    std::vector<std::string> passOrder = {"equirectangularToCubemap", "irradiance","prefilter","brdf","SSAOGeometry", "SSAO","shadowMappingDepth","pbr","background"};
//    std::vector<std::string> passOrder = {"equirectangularToCubemap", "irradiance","prefilter","brdf","shadowMappingDepth","pbr","background"};
    passManager.setPassOrder(passOrder);

    return STATE_OK;
}

int RenderClient::Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) {
//    LOGI("RenderClient update");
    glm::mat4 mProject = project;
    glm::mat4 mView = view;

    glm::mat4 model_trans_mat = glm::mat4(1.0);

    if(sceneData.actionLock.try_lock()){
        if(!sceneData.actionPassage.isEmpty()){
            std::string &modelName = sceneData.actionPassage.modelName;
            std::string &instanceName = sceneData.actionPassage.instanceName;
            std::string &originState = sceneData.actionPassage.originState;
            std::string &targetState = sceneData.actionPassage.targetState;
            //设置活跃模型
            cadDataManager::DataInterface::setActiveDocumentData(modelName);

            auto animationState = cadDataManager::DataInterface::getAnimationState(modelName, instanceName);
            std::vector<cadDataManager::AnKeyframe> &anKeyframes = animationState->keyframes;
            for (auto& anKeyframe : anKeyframes) {
                if (anKeyframe.originState == originState && anKeyframe.targetState == targetState) {
                    //TODO 找不到position因为动画Json数据对应关系没做好
                    positionArray = anKeyframe.positionArray;
                    quaternionArray = anKeyframe.quaternionArray;
                }
            }
        } else sceneData.actionLock.unlock();
    }


    std::vector<cadDataManager::AnimationActionUnit::Ptr> animationActions = cadDataManager::DataInterface::getAnimationActions("EngineFireAlarm");
    auto animationAction = animationActions[0];
    std::string modelName = animationAction->modelName;
    std::string instanceId = animationAction->instanceId;
    std::string originState = animationAction->originState;
    std::string targetState = "3";
//    LOGI("%s", modelName.c_str());
//    LOGI("%s", instanceId.c_str());
//    LOGI("%s", originState.c_str());
//    LOGI("%s", targetState.c_str());
    //加载了多个模型时，需要对ModelName模型执行动画，切换该ModelName为当前活跃状态
    cadDataManager::DataInterface::setActiveDocumentData(modelName);

    cadDataManager::AnimationStateUnit::Ptr animationState = cadDataManager::DataInterface::getAnimationState(modelName, instanceId);
    std::vector<cadDataManager::AnKeyframe> anKeyframes = animationState->keyframes;
    for (auto& anKeyframe : anKeyframes) {
        if (anKeyframe.originState == originState && anKeyframe.targetState == targetState) {
            //TODO 找不到position因为动画Json数据对应关系没做好
            positionArray = anKeyframe.positionArray;
            quaternionArray = anKeyframe.quaternionArray;
        }
    }

    //调用位姿变换接口，实时更新模型位置
    if (positionArray.size() == 0) {
        spdlog::error("未找到positionArray");
        LOGI("未找到positionArray");
    }
    else{
        actionFrame ++;
//        LOGI("%i", actionFrame);
        std::vector<float> position = { positionArray[actionFrame * 3], positionArray[actionFrame * 3 + 1], positionArray[actionFrame * 3 + 2] };
        std::vector<float> quaternion = { quaternionArray[actionFrame * 4] , quaternionArray[actionFrame * 4 + 1], quaternionArray[actionFrame * 4 + 2],quaternionArray[actionFrame * 4 + 3] };
        std::vector<float> matrix = cadDataManager::DataInterface::composeMatrix(position, quaternion);
        std::unordered_map<std::string, std::vector<cadDataManager::RenderInfo>> modifyModel = cadDataManager::DataInterface::modifyInstanceMatrix(instanceId, matrix);//零件实例位置的修改

//        mModel->getMMeshes().at(modifyModel.begin()->first).mTransformVector = modifyModel.begin()->second.begin()->matrix;

        std::string meshName = modifyModel.begin()->first;
        auto &meshTransform = modifyModel.begin()->second.begin()->matrix;
        mModel->updateMMesh(meshName, meshTransform);

        if((actionFrame * 3 + 3) == positionArray.size()){
            actionFrame = -1;
            sceneData.actionPassage.clear();
            positionArray.clear();
            quaternionArray.clear();
            sceneData.actionLock.unlock();
        }
    }
    //执行动画
//    mModel->getMMeshes()
    mModel->render(project,view,model_trans_mat);
    mPbrPass->render(project, view, joc);


    return STATE_OK;
}

int RenderClient::CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) {
    LOGI("Rendering CollectRemoteProcs frameDataPtr");
    return STATE_OK;
}

int RenderClient::ProRemoteReturn(RemoteProcPtr proc) {
    return STATE_OK;
}

int RenderClient::ShutDown(AppData& appData, SceneData& sceneData) {
    return STATE_OK;
}

void RenderClient::PreCompute(std::string configPath) {
}