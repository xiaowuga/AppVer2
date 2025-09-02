#include"InputRealSense/include/InputOffline.h"
#include <fstream>
#include <sstream>
#include <experimental/filesystem>

void remove_line_break(std::string& line){
    if(!line.empty() && (line.back() == '\n' || line.back() == '\r')){
        line.pop_back();
    }
}

class OfflineLoader {
public:
    std::vector<std::pair<double,std::string>> rgbImagePaths;
    std::vector<std::pair<double,std::string>> depthImagePaths;
    std::vector<std::vector<double>> imuData;
    int currentIndex;

    OfflineLoader(const std::string& datasetPath) : currentIndex(0) {
        // 读取图像路径
        std::ifstream rgbTimestampFile(datasetPath + "/cam0/cam_timestamp.txt");
        std::ifstream depthTimestampFile(datasetPath + "/cam1/cam_timestamp.txt");
        std::string line;

        while (std::getline(rgbTimestampFile, line)) {
            remove_line_break(line);
            double timestamp = std::strtod(line.c_str(),nullptr);
            rgbImagePaths.emplace_back(timestamp, datasetPath + "/cam0/data/" + line + ".png");
        }

        while (std::getline(depthTimestampFile, line)) {
            remove_line_break(line);
            double timestamp = std::strtod(line.c_str(),nullptr);
            depthImagePaths.emplace_back(timestamp, datasetPath + "/cam1/data/" + line + ".png");
        }

        // 读取IMU数据
        std::ifstream imuFile(datasetPath + "/imu0/data.csv");
        while (std::getline(imuFile, line)) {
            if (line[0] == '#') continue; // 跳过标题行
            std::vector<double> imuEntry(7, 0);
            std::istringstream ss(line);
            for (int i = 0; i < 7; ++i) {
                std::string val;
                if (std::getline(ss, val, ',')) {
                    imuEntry[i] = std::strtod(val.c_str(),nullptr);
                }
            }
            imuData.push_back(imuEntry);
        }
    }

    bool updateFrame(FrameDataPtr frameDataPtr, SceneData& sceneData) {
        if (currentIndex >= rgbImagePaths.size() || currentIndex >= depthImagePaths.size()) {
            return false;
        }
        frameDataPtr->imgColor = cv::imread(rgbImagePaths[currentIndex].second, cv::IMREAD_COLOR);
        frameDataPtr->imgDepth = cv::imread(depthImagePaths[currentIndex].second, cv::IMREAD_UNCHANGED);
        frameDataPtr->timestamp = rgbImagePaths[currentIndex].first;

        frameDataPtr->image.push_back(frameDataPtr->imgColor);
        frameDataPtr->depth.push_back(frameDataPtr->imgDepth);

        // convert depth for viz
        cv::Mat depth_abs;
        cv::convertScaleAbs(frameDataPtr->imgDepth, depth_abs, 0.03);  // 将深度图像转换为8位图像（即 cv::convertScaleAbs）
        cv::Mat depth_forViz;
        cv::applyColorMap(depth_abs, depth_forViz, cv::COLORMAP_JET);  // 使用 JET 颜色映射应用到深度图像上


        // // namedWindow("RGB", cv::WINDOW_AUTOSIZE);
        // cv::nameWindow("RGB", cv::NORMAL_WINDOW);
        // cv::imshow("RGB", frameDataPtr->imgColor);
        // cv::waitKey(1);
        // namedWindow("Depth", cv::WINDOW_AUTOSIZE);
        // // cv::imshow("Depth", frameDataPtr->imgDepth);
        // cv::imshow("Depth", depth_forViz);
        // cv::waitKey(1);

        // std::cout << "---------[main] Frame: " << std::fixed << std::setprecision(6) << frameDataPtr->timestamp << std::endl;

        // 更新IMU数据
        auto it = imuData.begin();
        while (it != imuData.end()) {
            if ((*it)[0] < frameDataPtr->timestamp) {
                // std::cout << "[main] IMU: " << std::fixed << std::setprecision(6) << (*it)[0] << std::endl;

                sceneData.imuBuffer.push_back(*it);
                it = imuData.erase(it);
            } else {
                break;
            }
        }
        ++currentIndex;
        return true;
    }
};

InputOffline::InputOffline() : _impl(nullptr) {}

int InputOffline::Init(AppData& appData, SceneData& sceneData,  FrameDataPtr frameDataPtr) {
    std::string datasetPath = appData.offlineDataDir;
    _impl = std::make_shared<OfflineLoader>(datasetPath);
    return STATE_OK;
}

int InputOffline::Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) {
    return _impl->updateFrame(frameDataPtr, sceneData) ? STATE_OK : STATE_ERROR;
}

int InputOffline::ShutDown(AppData& appData, SceneData& sceneData) {
    // waitting impl
    std::cout << "Module InputOffline quit" << std::endl;
    return STATE_OK;
}