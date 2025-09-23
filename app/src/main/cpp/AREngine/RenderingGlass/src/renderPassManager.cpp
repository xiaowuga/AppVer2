#include "renderPassManager.h"
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
//            pass->render(p, v,m);
            glm::mat4 pTemp = glm::mat4(
                    2.73653078f, 0.000000f, 0.000000f, 0.000000f,
                    0.000000f, 4.37845373f, 0.000000f, 0.000000f,
                    -0.0229938533f, 0.394046187f, -1.00100052f, -1.000000f,
                    0.000000f, 0.000000f, 0.000000f, 1.000000f  // 假设的第四行
            );

            // 定义视图矩阵 v
            glm::mat4 vTemp = glm::mat4(
                    0.524100542f, -0.215759903f, 0.823872745f, 0.000000f,
                    -0.00834672898f, 0.966029167f, 0.258298248f, 0.000000f,
                    -0.851615488f, -0.142250896f, 0.504495502f, 0.000000f,
                    0.141949967f, -0.124221973f, -0.094625391f, 1.000000f
            );

            // 定义模型矩阵 m
            glm::mat4 mTemp = glm::mat4(
                    0.481512f, 0.130643f, 0.866648f, -0.602090f,
                    -0.005659f, 0.989271f, -0.145984f, -0.017990f,
                    -0.876421f, 0.065389f, 0.477085f, -0.346697f,
                    0.000000f, 0.000000f, 0.000000f, 1.000000f
            );
            mTemp = glm::transpose(mTemp);
            pass->render(p, v, m); // 使用单位矩阵作为模型矩阵
        }
    }
}

