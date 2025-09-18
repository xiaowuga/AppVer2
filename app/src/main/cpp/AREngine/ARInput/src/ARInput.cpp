#include "ARInput.h"
#include "json.hpp"

#include <regex>
#include <string>
using namespace cv;

ARInputSources* ARInputSources::instance() {
    static ARInputSources *ptr=nullptr;
    if(!ptr)
        ptr=new ARInputSources();
    return ptr;
}

void ARInputSources::set(const ARInputSources::FrameData &frameData, int mask) {
    std::unique_lock<std::shared_mutex> _lock(_dataMutex);
    if(mask&DATAF_IMAGE)        _frameData.img=frameData.img;
    if(mask&DATAF_TIMESTAMP)    _frameData.timestamp=frameData.timestamp;
    if(mask&DATAF_CAMERAMAT)    _frameData.cameraMat=frameData.cameraMat;
}

void ARInputSources::get(ARInputSources::FrameData &frameData, int mask) {
    std::shared_lock<std::shared_mutex> _lock(_dataMutex);
    frameData=_frameData;
}

std::string prase_path(const std::string& str) {
    std::regex pattern(R"((.*)<\d+>)");

    // 提取并匹配
    std::smatch match;
    if (std::regex_match(str, match, pattern)) {
        return match[1];
    } else {
        return str;
    }
}

int ARInputs::Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) {

    nlohmann::json instances_info_json;
    std::ifstream json_file(appData.dataDir + "InstanceInfo.json");
    json_file >> instances_info_json;
    // [ {"instanceId": string, "name": string, "matrixWorld": [float]}, {...}, ...]
    for (int i = 0; i < instances_info_json.size(); i++) {
        auto instance_info_json = instances_info_json[i];
        std::string object_name = instance_info_json.at("name").get<std::string>();

        std::vector<float> model_mat = instance_info_json.at(
                "matrixWorld").get<std::vector<float>>();
        if (model_mat.size() != 16) {
            std::cout << "number of elements in model matrix does not equal to 16!" << std::endl;
        }
//        assert(model_mat.size() != 16);
        //                glm::mat4 model_mat_glm = glm::make_mat4(model_mat.data());
        cv::Matx44f model_mat_cv;
        std::copy(model_mat.begin(), model_mat.begin() + 16, model_mat_cv.val);
        std::string model_name = prase_path(object_name);
        std::string mesh_file_name = appData.dataDir + "Models/" + model_name + "/" + model_name + ".obj";
        Pose transform(model_mat_cv);
        Pose initTransform(cv::Matx44f::eye());
        SceneObjectPtr ptr = std::make_shared<SceneObject>(object_name, mesh_file_name,initTransform, transform);
        sceneData.setObject(object_name, ptr);
    }

    return STATE_OK;
};

int ARInputs::Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr)
{
    ARInputSources::FrameData frameData;
    ARInputSources::instance()->get(frameData);

    while(frameData.timestamp==this->_lastTimestamp)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        ARInputSources::instance()->get(frameData);
    }
    _lastTimestamp=frameData.timestamp;

    if(!frameData.img.empty())
    {
        cv::Mat img=frameData.img, dst;

        if(_udistMap1.empty())
        {
            cv::Matx33f K = { 281.60213015, 0.,  318.69481832,  0., 281.37377039, 243.6907021, 0., 0., 1. };
            cv::Mat distCoeffs = (cv::Mat_<float>(1, 4) << 0.11946399, 0.06202764, -0.28880297, 0.21420146);

            Mat newK;
            cv::fisheye::estimateNewCameraMatrixForUndistortRectify(K, distCoeffs, img.size(), noArray(), newK, 0.f);

            cv::fisheye::initUndistortRectifyMap(K, distCoeffs, noArray(), newK, img.size(), CV_16SC2, _udistMap1, _udistMap2);
            _camK=newK;
        }

        cv::remap(img, dst, _udistMap1, _udistMap2, INTER_LINEAR);
        img=dst;

        frameDataPtr->image.push_back(img);
        frameDataPtr->colorCameraMatrix=_camK;
    }

    sceneData.setData("ARInputs", frameData);
    return STATE_OK;
}


