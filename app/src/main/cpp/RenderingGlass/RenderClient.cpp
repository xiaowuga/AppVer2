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

#define LOG_TAG "RenderClient.cpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define HAND_JOINT_COUNT 16

RenderClient::RenderClient() = default;

RenderClient::~RenderClient() = default;

//TODO：现在没有传进来的关节位姿，后续需要添加

int RenderClient::Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) {
    LOGI("RenderClient init");

    mModel  = std::make_shared<renderModel>("test");
    mModel->loadFbModel(appData.dataDir + "Models/YIBIAOPAN.fb");

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
    LOGI("RenderClient update");;
    glm::mat4 mProject = glm::mat4(1.0);
    glm::mat4 mView = glm::mat4(1.0);
    glm::mat4 relocMatrix = frameDataPtr->modelRelocMatrix;
    glm::mat4 model_trans_mat = glm::mat4(1.0);


    // 旋转角度
    float angleX = glm::radians(-90.0f);
    float angleY = glm::radians(-90.0f);
    float angleZ = glm::radians(0.0f);

    // 分别绕各轴旋转
    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), angleX, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), angleY, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), angleZ, glm::vec3(0.0f, 0.0f, 1.0f));

    // 合并旋转（注意顺序）
    glm::mat4 rotationMatrix = rotationX * rotationY * rotationZ;


    glm::mat4 translateMatrix = glm::translate(model_trans_mat, glm::vec3(0.0,0.0, -1.0));
    model_trans_mat =  relocMatrix * translateMatrix * rotationMatrix;
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