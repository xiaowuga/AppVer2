//
// Created by shutiao on 2025/9/13.
//

#include "RenderClient.h"
#include <fstream>
#include <jni.h>
#include"opencv2/core.hpp"
#include "app/utilsmym.hpp"
#include <opencv2/opencv.hpp>
#include <utility>

#include <android/log.h>
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
    mTextRender = std::make_shared<Text>();
    mTextRender->initialize();

    auto scene_virtualObjects = sceneData.getAllObjectsOfType<SceneModel>();

    model_transforms_vector.reserve(scene_virtualObjects.size());
    for(int i = 0; i < scene_virtualObjects.size(); i ++){
        mModel->loadFbModel(scene_virtualObjects[i]->name, scene_virtualObjects[i]->filePath);
        model_transforms_vector.push_back({
           1,0,0,0,
           0,1,0,0,
           0,0,1,0,
           0,0,0,1
        });
        model_paths.push_back(scene_virtualObjects[i]->filePath);
        instance_names.push_back(scene_virtualObjects[i]->name);
    }

    sceneData.baseModelCount = (int)scene_virtualObjects.size();

    //加载模型的动画数据：Action + State
    cadDataManager::DataInterface::loadAnimationActionData(appData.animationActionConfigFile);
    cadDataManager::DataInterface::loadAnimationStateData(appData.animationStateConfigFile);

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
//    std::vector<std::string> passOrder = {
//            "equirectangularToCubemap",
//            "irradiance",
//            "prefilter",
//            "brdf",
//            "SSAOGeometry",
//            "SSAO",
//            "shadowMappingDepth",
//            "pbr",
//            "background"};
    std::vector<std::string> passOrder = {"equirectangularToCubemap", "irradiance","prefilter","brdf","shadowMappingDepth","pbr","background"};
    passManager.setPassOrder(passOrder);

    startTime = std::chrono::high_resolution_clock::now();

    mGizmoPass = std::make_shared<GizmoPass>();
    mGizmoPass->initBoundingBoxMap(boundingBoxMap);
    return STATE_OK;
}

int RenderClient::Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) {
//    LOGI("RenderClient update");
    if(sceneData.render_init_done) {


        // ===== 仪表盘 instance 筛选（仅执行一次）=====
        static bool dashboard_instances_collected = false;
        if (!dashboard_instances_collected) {
            dashboard_instances_collected = true;
            std::lock_guard<std::mutex> lock(sceneData.renderUploadLock);

            cadDataManager::DataInterface::setActiveDocumentData("YIBIAOPAN");
            auto MapInfo = cadDataManager::DataInterface::getRenderInfoMap();

            for (auto it = MapInfo.begin(); it != MapInfo.end(); ++it) {
                auto& renderInfos = it->second;
                for (auto& renderInfo : renderInfos) {
                    auto& instance_ids = renderInfo.instanceIds;
                    auto& mat = renderInfo.matrix;

                    for (size_t m_i = 0; m_i < mat.size() / 16; m_i++) {
                        std::string instance_id = instance_ids[m_i];

                        // 筛选 ID 以 "52" 开头的 instance
                        if (instance_id.rfind("52", 0) == 0) {
                            std::vector<double> matrix_16(mat.begin() + m_i * 16, mat.begin() + m_i * 16 + 16);

                            sceneData.ybp_names.push_back("YIBIAOPAN"+instance_id);
                            sceneData.ybp_array.push_back(matrix_16);
                        }
                    }
                }
            }

        }

        glm::mat4 mProject = project;
        glm::mat4 mView = view;

        glm::mat4 model_trans_mat = glm::mat4(1.0);

        //交互需要的接口：仅在当前无动画播放时才读取新的actionPassage
        if (actionFrame < 0 && !sceneData.actionPassage.isEmpty()) {
            state = true;
            modelName = sceneData.actionPassage.modelName;
            instanceName = sceneData.actionPassage.instanceName;
            originState = sceneData.actionPassage.originState;
            targetState = sceneData.actionPassage.targetState;
            instanceId = sceneData.actionPassage.instanceId;

            // 读取后立即清空，防止重复触发
            {
                std::lock_guard<std::mutex> lock(sceneData.actionLock);
                sceneData.actionPassage.clear();
            }

            //设置活跃模型
            cadDataManager::DataInterface::setActiveDocumentData(modelName);
            auto animationState = cadDataManager::DataInterface::getAnimationStateByName(modelName,
                                                                                         instanceName);
            std::vector<cadDataManager::AnKeyframe> &anKeyframes = animationState->keyframes;
            for (auto &anKeyframe: anKeyframes) {
                if (anKeyframe.originState == originState &&
                    anKeyframe.targetState == targetState) {
                    positionArray = anKeyframe.positionArray;
                    quaternionArray = anKeyframe.quaternionArray;
                }
            }
            // 仅在没有找到动画脚本时才高亮零件（姿态正确但无对应动画，用于指引操作者）
            if (positionArray.empty()) {
                if (!isHighLight[instanceName]) {
                    highlightInstance(modelName, instanceId);
                    isHighLight[instanceName] = true;
                }
            }


        }

//    {//测试接口用代码，推力杆会动
//        std::vector<cadDataManager::AnimationActionUnit::Ptr> animationActions = cadDataManager::DataInterface::getAnimationActions("EngineFireAlarm");
//        auto animationAction = animationActions[0];
//        modelName = animationAction->modelName;
//        instanceName = animationAction->instanceName;
//        instanceId = animationAction->instanceId;
//        originState = animationAction->originState;
//        targetState = "3";
//        //加载了多个模型时，需要对ModelName模型执行动画，切换该ModelName为当前活跃状态
//        cadDataManager::DataInterface::setActiveDocumentData(modelName);
//        auto instance = cadDataManager::DataInterface::getInstanceByName(instanceName);
//        instanceId = instance->getId();
//
//        cadDataManager::AnimationStateUnit::Ptr animationState = cadDataManager::DataInterface::getAnimationStateByName(modelName, instanceName);
//        std::vector<cadDataManager::AnKeyframe> anKeyframes = animationState->keyframes;
//        for (auto& anKeyframe : anKeyframes) {
//            if (anKeyframe.originState == originState && anKeyframe.targetState == targetState) {
//                positionArray = anKeyframe.positionArray;
//                quaternionArray = anKeyframe.quaternionArray;
//            }
//        }
//    }

        // 计算经过的时间
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsedTime = currentTime - startTime;
        stepTime += elapsedTime.count();

        //调用位姿变换接口，实时更新模型位置
        if (positionArray.size() != 0) {
            LOGI("找到positionArray， %s", modelName.c_str());
            actionFrame++;
//        LOGI("%i", actionFrame);
            std::vector<float> position = {positionArray[actionFrame * 3],
                                           positionArray[actionFrame * 3 + 1],
                                           positionArray[actionFrame * 3 + 2]};
            std::vector<float> quaternion = {quaternionArray[actionFrame * 4],
                                             quaternionArray[actionFrame * 4 + 1],
                                             quaternionArray[actionFrame * 4 + 2],
                                             quaternionArray[actionFrame * 4 + 3]};
            std::vector<float> matrix = cadDataManager::DataInterface::composeMatrix(position,
                                                                                     quaternion);

            // 先恢复高亮颜色（删除多余的高亮零件），再执行动画
            if (isHighLight[instanceName]) {
                // 记录恢复前每个proto的mesh数量，用于后续删除高亮多余的mesh
                std::unordered_map<std::string, int> oldMeshCounts = mModel->protoId;

                auto restoreModel = cadDataManager::DataInterface::restoreInstanceColor(instanceId);
                mModel->processMeshData(restoreModel);

                // 删除高亮时多出来的mesh（恢复后的数量 < 恢复前的数量）
                for (auto& kv : restoreModel) {
                    auto& protoName = kv.first;
                    int newCount = (int)kv.second.size();
                    auto oldIt = oldMeshCounts.find(protoName);
                    if (oldIt != oldMeshCounts.end() && newCount < oldIt->second) {
                        for (int i = newCount; i < oldIt->second; i++) {
                            mModel->getMMeshes()->erase(protoName + std::to_string(i));
                        }
                    }
                }

                isHighLight[instanceName] = false;
            }

            std::unordered_map<std::string, std::vector<cadDataManager::RenderInfo>> modifyModel;
            modifyModel = cadDataManager::DataInterface::modifyInstanceMatrix(instanceId,
                                                                              matrix);//零件实例位置的修改
            mModel->processMeshData(modifyModel);


            cadDataManager::DataInterface::setActiveDocumentData(modelName);
            auto MapInfo = cadDataManager::DataInterface::getRenderInfoMap();

            for (auto it = MapInfo.begin(); it != MapInfo.end(); ++it) {
                std::lock_guard<std::mutex> lock(sceneData.renderUploadLock);
                auto& renderInfos = it->second;
                for (auto& renderInfo : renderInfos) {
                    auto& instance_ids = renderInfo.instanceIds;
                    auto& mat = renderInfo.matrix;

                    for (size_t m_i = 0; m_i < mat.size() / 16; m_i++) {
                        std::string temp_instance_id = instance_ids[m_i];

                        // 筛选 instanceId
                        if (temp_instance_id == instanceId) {
                            std::vector<double> matrix_16(mat.begin() + m_i * 16, mat.begin() + m_i * 16 + 16);

                            sceneData.animation_names_buffer.push_back(modelName+instanceId);
                            sceneData.animation_transforms_buffer.push_back(matrix_16);
                        }
                    }
                }
                sceneData.hasAnimationData = true;
            }




            // push animation data to sceneData for RenderUpload (cross-thread safe)
//            std::vector<double> animTransform = {
//                    matrix[0], matrix[1], matrix[2], matrix[3],
//                    matrix[4], matrix[5], matrix[6], matrix[7],
//                    matrix[8], matrix[9], matrix[10], matrix[11],
//                    matrix[12], matrix[13], matrix[14], matrix[15]
//            };
//            std::string animName = modelName + instanceId;
//
//            {
//                std::lock_guard<std::mutex> lock(sceneData.renderUploadLock);
//                sceneData.animation_transforms_buffer.push_back(animTransform);
//                sceneData.animation_names_buffer.push_back(animName);
//                sceneData.hasAnimationData = true;
//            }

            if ((actionFrame * 3 + 3) == positionArray.size()) {
                actionFrame = -1;
                positionArray.clear();
                quaternionArray.clear();

                // animation ended, clear animation buffer in sceneData
                {
                    std::lock_guard<std::mutex> lock(sceneData.renderUploadLock);
                    sceneData.animation_transforms_buffer.clear();
                    sceneData.animation_names_buffer.clear();
                    sceneData.hasAnimationData = false;
                }
            }
        }

        mModel->render(project, view, model_trans_mat);
        mGizmoPass->updateBoundingBOX(boundingBoxArray);
//    mGizmoPass->render(project, view);
        mPbrPass->render(project, view, joc);

        wchar_t text[1024] = {0};
        std::string fps_str = std::to_string(int(getFps()));
        swprintf(text, 1024, L"fps:%s", fps_str.c_str());
        mTextRender->render(0.5, 0.5, 1.0, text, wcslen(text), glm::vec3(0.0, 1.0, 0.0));

//        updateFrameCount();
//    auto testNum = getFps();
//    testNum = getIndiceSum();
//    testNum = getFps();
        if (appData.environmentalState != environmentalState) {
            environmentalState = appData.environmentalState;
//        auto& passManager = RenderPassManager::getInstance();
//        auto pbrPass = passManager.getPassAs<PbrPass>("pbr");
//        pbrPass->setLightChange(true);

            auto &passManager = RenderPassManager::getInstance();
            auto equiPass = passManager.getPassAs<EquirectangularToCubemapPass>(
                    "equirectangularToCubemap");
            equiPass->setEnvCubeMap(environmentalState);

            auto irradiancePass = passManager.getPassAs<IrradiancePass>("irradiance");
            irradiancePass->setIrradianceMap(environmentalState);

            auto prefilterPass = passManager.getPassAs<PrefilterPass>("prefilter");
            prefilterPass->setPrefilterMap(environmentalState);
        }
    }
    startTime = std::chrono::high_resolution_clock::now();
    return STATE_OK;
}

int RenderClient::CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) {
    LOGI("Rendering CollectRemoteProcs frameDataPtr");
    return STATE_OK;
}

int RenderClient::ProRemoteReturn(RemoteProcPtr proc) {
    LOGI("init  done !!!");
    auto& send = proc->send;
    auto& ret = proc->ret;
    auto cmd = send.getd<std::string>("cmd");

    if (cmd == "initCloudRenderer") {
        auto init_done = ret.getd<bool>("init_done");

        if (init_done) {
            LOGI("init  done !!!");

            render_init_done = true;
        }
    }
    return STATE_OK;
}

int RenderClient::ShutDown(AppData& appData, SceneData& sceneData) {
    return STATE_OK;
}

void RenderClient::PreCompute(std::string configPath) {
}

void RenderClient::updateFrameCount() {

    // 增量帧计数器
    frameCount++;

    // 计算经过的时间
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsedTime = currentTime - startTime;

    // 检查是否已过一秒
    if (elapsedTime.count() >= 1.0f)
    {
        // 计算 FPS
        fps = frameCount / elapsedTime.count();

        // 将FPS输出到控制台
        std::cout << "FPS: " << fps << std::endl;

        // 下一秒重置
        frameCount = 0;
        startTime = currentTime;
    }
}

void RenderClient::highlightInstance(std::string modelName, std::string instanceId){
    std::unordered_map<std::string, std::vector<cadDataManager::RenderInfo>> modifyModel;
    cadDataManager::DataInterface::setActiveDocumentData(modelName);
    modifyModel = cadDataManager::DataInterface::modifyInstanceColor(instanceId, "#FF0000");//零件实例高亮的修改
    mModel->processMeshData(modifyModel);
}

std::vector<float> countCenter(){

}