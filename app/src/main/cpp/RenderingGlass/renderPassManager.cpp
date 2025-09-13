#include "RenderingGlass/renderPassManager.h"
#include <stdexcept>
#include <GLES3/gl3.h>

void RenderPassManager::registerPass(const std::string& name, std::shared_ptr<TemplatePass> pass) {
    mPasses[name] = pass;
}

std::shared_ptr<TemplatePass> RenderPassManager::getPass(const std::string& name) const {
    auto it = mPasses.find(name);
    if (it != mPasses.end()) {
        return it->second;
    }
    throw std::runtime_error("Render pass not found: " + name);
}

void RenderPassManager::setPassOrder(const std::vector<std::string>& order) {
    // 验证所有的pass名称都已注册
    for (const auto& passName : order) {
        if (mPasses.find(passName) == mPasses.end()) {
            throw std::runtime_error("Cannot set pass order: Pass not registered: " + passName);
        }
    }
    mPassOrder = order;
}

void RenderPassManager::saveViewport() {
    // 保存当前的viewport设置
    glGetIntegerv(GL_VIEWPORT, mSavedViewport);
}

void RenderPassManager::restoreViewport() {
    // 恢复之前保存的viewport设置
    glViewport(mSavedViewport[0], mSavedViewport[1], mSavedViewport[2], mSavedViewport[3]);
}

void RenderPassManager::executeAllPasses(const glm::mat4& p, const glm::mat4& v) {
    // 首先保存当前的viewport
    saveViewport();

    // 执行所有Pass
    if (mPassOrder.empty()) {
        // 如果没有设置执行顺序，则按注册顺序执行（不保证顺序）
        for (const auto& [name, pass] : mPasses) {
            pass->render(p, v, glm::mat4(1.0f)); // 使用单位矩阵作为模型矩阵
        }
    } else {
        // 按指定顺序执行
        for (const auto& passName : mPassOrder) {
            auto pass = mPasses.at(passName);
            pass->render(p, v);
        }
    }
}
void RenderPassManager::executeAllPasses(const glm::mat4& p, const glm::mat4& v, const glm::mat4& m) {
    // 首先保存当前的viewport
    saveViewport();

    // 执行所有Pass
    if (mPassOrder.empty()) {
        // 如果没有设置执行顺序，则按注册顺序执行（不保证顺序）
        for (const auto& [name, pass] : mPasses) {
            pass->render(p, v, m); // 使用单位矩阵作为模型矩阵
        }
    } else {
        // 按指定顺序执行
        for (const auto& passName : mPassOrder) {
            auto pass = mPasses.at(passName);
            pass->render(p, v,m);
        }
    }
}

