#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <typeindex>
#include <typeinfo>
#include "templatePass.h"

class RenderPassManager {
public:
    // 单例模式
    static RenderPassManager& getInstance() {
        static RenderPassManager instance;
        return instance;
    }

    // 注册渲染通道
    void registerPass(const std::string& name, std::shared_ptr<TemplatePass> pass);

    // 获取渲染通道
    std::shared_ptr<TemplatePass> getPass(const std::string& name) const;

    // 泛型方法，获取特定类型的Pass
    template<typename T>
    std::shared_ptr<T> getPassAs(const std::string& name) const {
        auto pass = getPass(name);
        if (!pass) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<T>(pass);
    }

    // 设置Pass执行顺序
    void setPassOrder(const std::vector<std::string>& order);

    // 执行所有Pass
    void executeAllPasses(const glm::mat4& p, const glm::mat4& v);
    void executeAllPasses(const glm::mat4 &p, const glm::mat4 &v, const glm::mat4 &m);
    // 获取Pass的输出纹理ID
    template<typename T>
    unsigned int getPassTextureOutput(const std::string& name, unsigned int (T::*getterFunc)() const) const {
        auto passTyped = getPassAs<T>(name);
        if (!passTyped) {
            throw std::runtime_error("Pass not found or cannot be cast to specified type: " + name);
        }
        return (passTyped.get()->*getterFunc)();
    }

    // 保存当前AR眼镜的viewport
    void saveViewport();

    // 恢复到AR眼镜的viewport
    void restoreViewport();

private:
    RenderPassManager() = default;
    ~RenderPassManager() = default;
    RenderPassManager(const RenderPassManager&) = delete;
    RenderPassManager& operator=(const RenderPassManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<TemplatePass>> mPasses;
    std::vector<std::string> mPassOrder; // pass执行顺序

    // 保存AR眼镜的viewport
    int mSavedViewport[4] = {0, 0, 0, 0}; // x, y, width, height

};

