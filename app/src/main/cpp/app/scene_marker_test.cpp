#include"scene.h"
#include"opencv2/opencv.hpp"
#include<opencv2/calib3d.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include"glm/gtx/quaternion.hpp"
#include"demos/model.h"
#include<common/xr_linear.h>
#include"demos/utils.h"
#include"scenegui.h"
#include"utilsmym.hpp"

#include <android/log.h>
#define LOG_TAG "native-lib"
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)



//inline std::string GlmMat4_to_String(const glm::mat4 &matrix) {
//    std::ostringstream oss;
//    for (int i = 0; i < 4; ++i) {
//        for (int j = 0; j < 4; ++j) {
//            oss << matrix[i][j];
//            if (j < 3) { oss << " "; }
//        }
//        if (i < 3) { oss << "\n"; }
//    }
//    return oss.str();  // 返回构建的字符串
//}
//inline glm::mat4 CV_Matx44f_to_GLM_Mat4(const cv::Matx44f &mat) {  // 将 cv::Matx44f 转换为 glm::mat4
//    return glm::mat4(
//            mat(0, 0), mat(0, 1), mat(0, 2), mat(0, 3),  // 第一行
//            mat(1, 0), mat(1, 1), mat(1, 2), mat(1, 3),  // 第二行
//            mat(2, 0), mat(2, 1), mat(2, 2), mat(2, 3),  // 第三行
//            mat(3, 0), mat(3, 1), mat(3, 2), mat(3, 3)   // 第四行
//    );
//}
//inline std::string MakeSdcardPath(const std::string &path) {
//    std::string res = path;
//    if (!path.empty() && path[0] != '/') res = "/storage/emulated/0/" + res;
//    return res;
//}


class SceneMarkerTest : public IScene {
    IOpenXrProgram::MarkerData m_markerData;
    typedef std::tuple<Model, glm::mat4> ModelItemType;
    std::vector<ModelItemType> modelList; //<Model, ModelTransMat>
    float mDefaultScale = 1.0f;//0.011f;
    std::shared_ptr<SceneGui> mGui;


public:
    bool add_model_from_file(const std::string &model_name, const std::string &file_name) {
        bool res = true;
        auto local_path = MakeSdcardPath(file_name);
        Model model(model_name);
        model.initialize();
        if (!local_path.empty()) {
            res = model.loadLocalModel(local_path);
            if (res) {
//            modelList.emplace_back(model,glm::mat4(1.0f));
                infof("Load Local Model Success: %s",local_path.c_str());
            }
            else errorf("Load Local Model Failed: %s",local_path.c_str());
            modelList.emplace_back(model, glm::mat4(1.0f));
        } else res = false;
        return res;
    }

    bool set_model_trans_pose(const float pose[7], int pos = -1) {
        bool flag = false;
        XrMatrix4x4f result;
        XrQuaternionf quat = {pose[0], pose[1], pose[2], pose[3]};
        XrMatrix4x4f_CreateFromQuaternion(&result, &quat);
        XrVector3f t = {pose[4], pose[5], pose[6]};
        XrVector3f scale = {1.f, 1.f, 1.f};
        XrMatrix4x4f_CreateTranslationRotationScale(&result, &t, &quat, &scale);

        cv::Matx44f m;
        //GetGLModelView(R, t, m.val, true);
        memcpy(m.val, result.m, sizeof(float) * 16);
        auto trans_mat = CV_Matx44f_to_GLM_Mat4(m);

        std::stringstream ss;
        ss << "Pose to Mat4: " << GlmMat4_to_String(trans_mat) << std::endl;
        infof(ss.str().c_str());
        ss.clear();


        LOGD("Pose to Mat4:\n%s", GlmMat4_to_String(trans_mat).c_str());

//        ss<<"ViewMat: "<<GlmMat4_to_String(ViewMat)<<std::endl;
//        infof(ss.str().c_str());

        for (int i = 0; i < (int) modelList.size(); ++i) {
            if (i != pos && pos != -1) continue;
            std::get<1>(modelList[i]) = trans_mat;
            flag = true;
        }
        return flag;
    }
//================================ Override ========================================
public:
    bool initialize(const XrInstance instance, const XrSession session) override {

//**************************************************************************************************add GUI

        mGui=std::make_shared<SceneGui>();
        int height=360;
        int width=480;

        glm::mat4 model_translate = glm::mat4(1.0f);
        float scale = 1.0f;
        scale = 0.7;
        model_translate = glm::translate(model_translate, glm::vec3(-0.0f, -0.3f, -1.0f));
        model_translate = glm::rotate(model_translate, glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model_translate = glm::scale(model_translate, glm::vec3(scale * (width / height), 0.7, 1.0f));

        glm::mat4  fix_on_cam = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

        std::vector<std::string> info_list = {"helloworld", "Rokid"};
        mGui->add_gui(width,height,"dashboard",model_translate,fix_on_cam,info_list);

//**************************************************************************************************endof add GUI

        std::vector<IOpenXrProgram::MarkerInfo> markers = {

                {"/storage/emulated/0/Pictures/sunflower.png", 0.268f, 0.152f}


        };
        int added_markers = m_program->AddMarkerImages(markers);
        if (added_markers != (int) markers.size()) {
            throw std::runtime_error("add marker failed");
        }
        add_model_from_file("MyModel", "Download/TextureModel/Mars/mars_obj.png");
        return true;
    }

    void inputEvent(int leftright, const ApplicationEvent &event) override {
        if(leftright==0){
            leftright=0;
        }
        std::stringstream ss;
        ss<<"GetEvent: LR: "<<leftright<<". Event: (thumbstick xy): ";
        ss<<event.thumbstick_x<<", "<<event.thumbstick_y;
        ss<<". Trigger: "<<event.trigger<<", Sqz: "<<event.squeeze;
        infof(ss.str().c_str());
    }

    void renderFrame(const XrPosef &pose, const glm::mat4 &project, const glm::mat4 &view,int32_t eye) override {
        for (int i = 0; i < (int) modelList.size(); ++i) {
            auto model = std::get<0>(modelList[i]);


//**********************************************************渲染UI
            mGui->render(project,view,eye);
//**********************************************************渲染UI

            model.render(project, view, std::get<1>(modelList[i]));
        }
    }

    void processFrame() override {
        m_program->ProcessMarkerData(m_markerData);
        LOGD("m_markerData SIZE: %d",m_markerData.updated.size());
        for (auto i: m_markerData.updated) {

            set_model_trans_pose(i.pose);
        }
    }

};

std::shared_ptr<IScene> _createScene_marker_test() {
    return std::make_shared<SceneMarkerTest>();
}




