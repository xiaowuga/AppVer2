//
// Created by shutiao on 2025/8/27.
//

#pragma once

#include <memory> // 用于std::unique_ptr
#ifndef ROKIDOPENXRANDROIDDEMO_RENDERINTERFACE_H
#define ROKIDOPENXRANDROIDDEMO_RENDERINTERFACE_H


// 前向声明所有Pass类，避免包含大量头文件
class TemplatePass;
class PbrPass;
class EquirectangularToCubemapPass;
class RenderPassManager;
class BrdfPass;
class IrradiancePass;
class PrefilterPass;
class BackgroundPass;
class ShadowMappingDepthPass;

class RenderInterface {
public:
    RenderInterface();
    ~RenderInterface(); // 如使用unique_ptr，析构函数需在实现文件中定义，而非默认 inline

    // 删除拷贝构造和赋值，防止多个实例管理同一份资源
    RenderInterface(const RenderInterface&) = delete;
    RenderInterface& operator=(const RenderInterface&) = delete;

    bool initialize(); // 初始化所有渲染Pass
    void render();     // 执行所有渲染Pass
    void cleanup();    // 清理资源（通常析构函数已足够，但也可提供显式清理）

    // 提供获取具体Pass的接口（根据需要）
    ShadowMappingDepthPass* getShadowMappingDepthPass() { return m_shadowMappingDepthPass.get(); }
    // ... 其他Pass的getter

private:
    // 使用std::unique_ptr管理对象生命周期，明确所有权[1](@ref)
    std::unique_ptr<PbrPass> m_pbrPass;
    std::unique_ptr<EquirectangularToCubemapPass> m_equirectangularToCubemapPass;
    std::unique_ptr<BrdfPass> m_brdfPass;
    std::unique_ptr<IrradiancePass> m_irradiancePass;
    std::unique_ptr<PrefilterPass> m_prefilterPass;
    std::unique_ptr<BackgroundPass> m_backgroundPass;
    std::unique_ptr<ShadowMappingDepthPass> m_shadowMappingDepthPass;
};

#endif //ROKIDOPENXRANDROIDDEMO_RENDERINTERFACE_H
