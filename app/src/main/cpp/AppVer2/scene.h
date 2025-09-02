#pragma once

#include"app/application.h"
#include<string>

class IScene {
public:
    IOpenXrProgram *m_program=nullptr;
    IApplication *m_app=nullptr;
public:
    virtual ~IScene() = default;
    virtual bool initialize(const XrInstance instance, const XrSession session) =0;
    virtual void inputEvent(int leftright, const ApplicationEvent& event) {}
    virtual void renderFrame(const XrPosef& pose, const glm::mat4& project, const glm::mat4& view, int32_t eye) {}
    virtual void processFrame() {}
    virtual void close() {}
};


std::shared_ptr<IScene> createScene(const std::string &name, IApplication *app);

