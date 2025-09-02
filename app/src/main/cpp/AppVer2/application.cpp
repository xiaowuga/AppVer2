#include <dirent.h>
#include "pch.h"
#include "common.h"
#include "options.h"
#include "application.h"
#include "controller.h"
#include "hand.h"
#include <common/xr_linear.h>
#include "logger.h"
#include "gui.h"
#include "ray.h"
#include "text.h"
#include "player.h"
#include "utils.h"
#include "graphicsplugin.h"
#include "cube.h"

class Application : public IApplication {
public:
    Application(const std::shared_ptr<struct Options>& options, const std::shared_ptr<IGraphicsPlugin>& graphicsPlugin);
    virtual ~Application() override;
    virtual void setControllerPose(int leftright, const XrPosef& pose) override;
    virtual bool initialize(const XrInstance instance, const XrSession session) override;
    virtual void setHandJointLocation(XrHandJointLocationEXT* location) override;
    virtual void inputEvent(int leftright, const ApplicationEvent& event) override;
    virtual void renderFrame(const XrPosef& pose, const glm::mat4& project, const glm::mat4& view, int32_t eye) override;
private:
    void layout();
    void showDashboard(const glm::mat4& project, const glm::mat4& view);
    void showDashboardController();
    void showDeviceInformation(const glm::mat4& project, const glm::mat4& view);
    void renderHandTracking(const glm::mat4& project, const glm::mat4& view);
    // Calculate the angle between the vector v and the plane normal vector n
    float angleBetweenVectorAndPlane(const glm::vec3& vector, const glm::vec3& normal);

private:
    std::shared_ptr<IGraphicsPlugin> mGraphicsPlugin;
    std::shared_ptr<Controller> mController;
    std::shared_ptr<Hand> mHandTracker;
    std::shared_ptr<Gui> mPanel;
    std::shared_ptr<Text> mTextRender;
    std::shared_ptr<Player> mPlayer;
    glm::mat4 mControllerModel;
    XrPosef mControllerPose[HAND_COUNT];
    std::shared_ptr<CubeRender> mCubeRender;

    //openxr
    XrInstance m_instance;          //Keep the same naming as openxr_program.cpp
    XrSession m_session;
    std::vector<XrView> m_views;
    float mIpd;
    XrHandJointLocationEXT m_jointLocations[HAND_COUNT][XR_HAND_JOINT_COUNT_EXT];

    //app data
    std::string mDeviceModel;
    std::string mDeviceOS;

    bool mIsShowDashboard = true;

    const ApplicationEvent *mControllerEvent[HAND_COUNT];

};

std::shared_ptr<IApplication> createApplication(const std::shared_ptr<struct Options>& options, const std::shared_ptr<IGraphicsPlugin>& graphicsPlugin) {
    return std::make_shared<Application>(options, graphicsPlugin);
}

Application::Application(const std::shared_ptr<struct Options>& options, const std::shared_ptr<IGraphicsPlugin>& graphicsPlugin) {
    mGraphicsPlugin = graphicsPlugin;
    mController = std::make_shared<Controller>();
    mHandTracker = std::make_shared<Hand>();
    mPanel = std::make_shared<Gui>("dashboard");
    mTextRender = std::make_shared<Text>();
    mPlayer = std::make_shared<Player>();
    mCubeRender = std::make_shared<CubeRender>();
}

Application::~Application() {
}

bool Application::initialize(const XrInstance instance, const XrSession session) {
    m_instance = instance;
    m_session = session;

    // get device model
    mDeviceModel = "Rokid AR Station";

    //get OS version
    //__system_property_get("ro.system.build.id", buffer); // You can also call this function, the result is the same
    mDeviceOS = "OpenXR";

    mController->initialize(mDeviceModel);
    mHandTracker->initialize(); // zhfzhf

    mPanel->initialize(600, 800);  //set resolution
    mTextRender->initialize();
    mCubeRender->initialize();

    const XrGraphicsBindingOpenGLESAndroidKHR *binding = reinterpret_cast<const XrGraphicsBindingOpenGLESAndroidKHR*>(mGraphicsPlugin->GetGraphicsBinding());
    mPlayer->initialize(binding->display);

    return true;
}


void Application::setControllerPose(int leftright, const XrPosef& pose) {
    XrMatrix4x4f model{};
    XrVector3f scale{1.0f, 1.0f, 1.0f};
    XrMatrix4x4f_CreateTranslationRotationScale(&model, &pose.position, &pose.orientation, &scale);
    glm::mat4 m = glm::make_mat4((float*)&model);
    mController->setModel(leftright, m);
    mHandTracker->setModel(leftright, m); // zhfzhf
    mControllerPose[leftright] = pose;
}
void Application::setHandJointLocation(XrHandJointLocationEXT* location) {
    memcpy(&m_jointLocations, location, sizeof(m_jointLocations));
}

void Application::inputEvent(int leftright, const ApplicationEvent& event) {
    mControllerEvent[leftright] = &event;

    if (event.controllerEventBit & CONTROLLER_EVENT_BIT_click_menu) {
        if (event.click_menu == true) {
            mIsShowDashboard = !mIsShowDashboard;
        }
    }

    if (leftright == HAND_LEFT) {
        return;
    }
    if (event.controllerEventBit & CONTROLLER_EVENT_BIT_click_trigger) {
        //infof("controllerEventBit:0x%02x, event.click_trigger:0x%d", event.controllerEventBit, event.click_trigger);
        mPanel->triggerEvent(event.click_trigger);
    }

}

void Application::layout() {
    glm::mat4 model = glm::mat4(1.0f);
    float scale = 1.0f;

    float width, height;
    mPanel->getWidthHeight(width, height);
    scale = 0.7;
    model = glm::translate(model, glm::vec3(-0.0f, -0.3f, -1.0f));
    model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(scale * (width / height), scale, 1.0f));
    mPanel->setModel(model);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.0f, -0.0f, -1.5f));
    model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(scale*2, scale, 1.0f));
    mPlayer->setModel(model);
}

void Application::showDashboardController() {
#define HAND_BIT_LEFT HAND_LEFT+1
#define HAND_BIT_RIGHT HAND_RIGHT+1
#define SHOW_CONTROLLER_ROW_float(x)    ImGui::TableNextRow();\
                                        ImGui::TableNextColumn();\
                                        ImGui::Text("%s", MEMBER_NAME(ApplicationEvent, x));\
                                        ImGui::TableNextColumn();\
                                        ImGui::Text("%f", mControllerEvent[HAND_LEFT]->x);\
                                        ImGui::TableNextColumn();\
                                        ImGui::Text("%f", mControllerEvent[HAND_RIGHT]->x);

#define SHOW_CONTROLLER_ROW_bool(hand, x)   ImGui::TableNextRow();\
                                            ImGui::TableNextColumn();\
                                            ImGui::Text("%s", MEMBER_NAME(ApplicationEvent, x));\
                                            ImGui::TableNextColumn();\
                                            if (hand & HAND_BIT_LEFT && mControllerEvent[HAND_LEFT]->x) {\
                                                ImGui::Text("true");\
                                            }\
                                            ImGui::TableNextColumn();\
                                            if (hand & HAND_BIT_RIGHT && mControllerEvent[HAND_RIGHT]->x) {\
                                                ImGui::Text("true");\
                                            }

}

void Application::showDashboard(const glm::mat4& project, const glm::mat4& view) {

    PlayModel playModel = mPlayer->getPlayStyle();

    const XrPosef& controllerPose = mControllerPose[1];
    glm::vec3 linePoint = glm::make_vec3((float*)&controllerPose.position);
    glm::vec3 lineDirection = mController->getRayDirection(1);
    mPanel->isIntersectWithLine(linePoint, lineDirection);

    mPanel->begin();
    if (ImGui::CollapsingHeader("information")) {
        ImGui::BulletText("device model: %s", mDeviceModel.c_str());
        ImGui::BulletText("device OS: %s", mDeviceOS.c_str());
    }
    
    //test controller
    showDashboardController();

    ImGui::Text("Please use the hand ray to pinch click.");

    mPanel->end();
    mPanel->render(project, view);

    mPlayer->setPlayStyle(playModel);
}

void Application::showDeviceInformation(const glm::mat4& project, const glm::mat4& view) {
    wchar_t text[1024] = {0};
    swprintf(text, 1024, L"model: %s, OS: %s", mDeviceModel.c_str(), mDeviceOS.c_str());

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.5f, -0.6f, -1.0f));
    model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5, 0.5, 1.0f));
    mTextRender->render(project, view, model, text, wcslen(text), glm::vec3(1.0, 1.0, 1.0));
}

float Application::angleBetweenVectorAndPlane(const glm::vec3& vector, const glm::vec3& normal) {
    float dotProduct = glm::dot(vector, normal);
    float lengthVector = glm::length(vector);
    float lengthNormal = glm::length(normal);
    if (lengthNormal != 1.0f) {
        lengthNormal = 1.0f;  //normalnize
    }
    float cosAngle = dotProduct / (lengthVector * lengthNormal);
    float angleRadians = std::acos(cosAngle);
    //Convert radians to degrees
    //float angleInDegrees = glm::degrees(angleRadians);
    return PI/2 - angleRadians;
}

//std::string BoneNames[2][26] {
//        {"p_l_palm", "p_l_wrist",
//            "p_l_thumb0", "p_l_thumb1", "p_l_thumb2", "p_l_thumb3",
//            "p_l_index1", "p_l_index2", "p_l_index3", "p_l_index_null", "p_l_index5",
//            "p_l_middle1", "p_l_middle2", "p_l_middle3", "p_l_middle_null", "p_l_middle5",
//            "p_l_ring1", "p_l_ring2", "p_l_ring3", "p_l_ring_null ", "p_l_ring5",
//            "p_l_pinky0", "p_l_pinky1", "p_l_pinky2", "p_l_pinky3","p_l_pinky_null",
//            },
//
//        {"p_r_palm", "p_r_wrist",
//         "p_r_thumb0", "p_r_thumb1", "p_r_thumb2", "p_r_thumb3",
//         "p_r_index1", "p_r_index2", "p_r_index3", "p_r_index_null", "p_r_index5",
//         "p_r_middle1", "p_r_middle2", "p_r_middle3", "p_r_middle_null", "p_r_middle5",
//         "p_r_ring1", "p_r_ring2", "p_r_ring3", "p_r_ring_null ", "p_r_ring5",
//         "p_r_pinky0", "p_r_pinky1", "p_r_pinky2", "p_r_pinky3","p_r_pinky_null",
//        },
//};

void Application::renderHandTracking(const glm::mat4& project, const glm::mat4& view) {
    std::vector<CubeRender::Cube> cubes;
    for (auto hand = 0; hand < HAND_COUNT; hand++) {
        for (int i = 0; i < XR_HAND_JOINT_COUNT_EXT; i++) {
            XrHandJointLocationEXT& jointLocation = m_jointLocations[hand][i];
            if (jointLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT && jointLocation.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT) {

                XrMatrix4x4f m{};
                XrVector3f scale{1.0f, 1.0f, 1.0f};
                XrMatrix4x4f_CreateTranslationRotationScale(&m, &jointLocation.pose.position, &jointLocation.pose.orientation, &scale);
                glm::mat4 model = glm::make_mat4((float*)&m);

                CubeRender::Cube cube;
                cube.model = model;
                cube.scale = 0.01f;
                cubes.push_back(cube);

                //mHandTracker->setBoneNodeMatrices(hand, getBoneNameByIndex(hand, i), model); // zhfzhf
            }
        }
        //mHandTracker->render(hand, project, view);
    }

    mCubeRender->render(project, view, cubes);
}

void Application::renderFrame(const XrPosef& pose, const glm::mat4& project, const glm::mat4& view, int32_t eye) {
    layout();
    showDeviceInformation(project, view);

    mPlayer->render(project, view, eye);

    if (mIsShowDashboard) {
        showDashboard(project, view);
    }

    mController->render(project, view);

    renderHandTracking(project, view);

}
