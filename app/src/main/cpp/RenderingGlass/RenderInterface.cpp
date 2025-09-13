//
// Created by shutiao on 2025/8/27.
//

#include "RenderingGlass/RenderInterface.h"
// RenderInterface.cpp
// 包含所有Pass的头文件
#include "RenderingGlass/templatePass.h"
#include "RenderingGlass/pbrPass.h"
#include "RenderingGlass/equirectangularToCubemapPass.h"
#include "RenderingGlass/renderPassManager.h"
#include "RenderingGlass/brdfPass.h"
#include "RenderingGlass/irradiancePass.h"
#include "RenderingGlass/prefilterPass.h"
#include "RenderingGlass/backgroundPass.h"
#include "RenderingGlass/shadowMappingDepthPass.h"

RenderInterface::RenderInterface()
        : m_pbrPass(std::make_unique<PbrPass>())
        , m_equirectangularToCubemapPass(std::make_unique<EquirectangularToCubemapPass>())
        , m_brdfPass(std::make_unique<BrdfPass>())
        , m_irradiancePass(std::make_unique<IrradiancePass>())
        , m_prefilterPass(std::make_unique<PrefilterPass>())
        , m_backgroundPass(std::make_unique<BackgroundPass>())
        , m_shadowMappingDepthPass(std::make_unique<ShadowMappingDepthPass>()) {
}

RenderInterface::~RenderInterface() = default; // 需在cpp中定义，因为unique_ptr的析构需要看到完整类型

bool RenderInterface::initialize() {
    // 按依赖顺序初始化各个Pass
    // 例如，IBL相关的Pass可能依赖于EquirectangularToCubemapPass的结果
    bool success = true;
//    success &= m_equirectangularToCubemapPass->initialize();
//    success &= m_brdfPass->initialize();
//    success &= m_irradiancePass->initialize();
//    success &= m_prefilterPass->initialize();
//    success &= m_shadowMappingDepthPass->initialize();
//    success &= m_templatePass->initialize();
//    success &= m_pbrPass->initialize();
//    success &= m_backgroundPass->initialize();
    // m_renderPassManager->initialize(); // 如果需要
    return success;
}

void RenderInterface::render() {
    // 1. 执行深度/阴影Pass（通常在渲染循环的开始）
//    m_shadowMappingDepthPass->render();

    // 2. 可能需要的环境光遮蔽或其它几何Pass
    // m_templatePass->render();

    // 3. 主渲染：PBR、背景等
//    m_pbrPass->render();
//    m_backgroundPass->render();

    // 4. 后处理或UI Pass（如果由RenderPassManager管理）
    // m_renderPassManager->executeAll();
}

void RenderInterface::cleanup() {
    // 按创建相反的顺序或依赖关系清理（通常unique_ptr析构会自动处理）
//    m_backgroundPass->cleanup();
//    m_pbrPass->cleanup();
//    m_templatePass->cleanup();
//    m_shadowMappingDepthPass->cleanup();
//    m_prefilterPass->cleanup();
//    m_irradiancePass->cleanup();
//    m_brdfPass->cleanup();
//    m_equirectangularToCubemapPass->cleanup();
    // m_renderPassManager->cleanup();
}

//TODO 1、创建接口类 2、创建新的scene，将application中实现渲染功能的代码迁移到scene 3、
