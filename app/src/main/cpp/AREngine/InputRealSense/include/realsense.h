#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include "Basic/include/BasicData.h"


using namespace std;
using namespace cv;

// namespace RSLoaderNamespace{
//     extern atomic<bool> running;
//     extern mutex imu_mutex;
// }

// atomic<bool> running(true);
// mutex imu_mutex;

class RSLoader{
    public:

        std::string base_path="";
        
        bool record_switch=false;

        vector<vector<double>> m_imu_save;

        vector<string> timestamp_pic;

        vector<vector<double>> m_imu_all;
        mutex m_imu_mutex;

        RSLoader(int width=640, int height=480, int fps=30);
        ~RSLoader();

        rs2::context m_ctx;
        rs2::device m_dev;

        rs2::pipeline m_pipe_image;
        rs2::config m_config_image;

        std::thread m_imu_thread;

        rs2::pipeline m_pipe_imu;
        rs2::config m_config_imu;
        bool imu_switch=true;

//        bool updateFrame(FrameData &framedate,SceneData &scenedata);
        bool updateFrame(AppData& appData,FrameDataPtr frameDataPtr,SceneData &scenedata);
        void imuThreadFunction();
        void save_images(const pair<double, Mat>& color, const pair<double, Mat>& depth);
        void initialize_files();
        void save_all_data();
        bool ShutDown();

};
