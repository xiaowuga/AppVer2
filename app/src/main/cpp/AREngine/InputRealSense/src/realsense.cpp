#include <iostream>
#include <iomanip>
#include <fstream>

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <chrono>

#include <chrono>

#include "InputRealSense/include/realsense.h"

using namespace std;
using namespace cv;

RSLoader::RSLoader(int width, int height, int fps) : m_pipe_image(m_ctx), m_pipe_imu(m_ctx)
{

    auto list = m_ctx.query_devices();
    if (list.size() == 0)
        throw std::runtime_error("No device detected.");
    m_dev = list.front();

    m_config_image.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_BGR8, fps);
    m_config_image.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);

    m_pipe_image.start(m_config_image);

    m_config_imu.enable_stream(RS2_STREAM_ACCEL, RS2_FORMAT_MOTION_XYZ32F, 200);
    m_config_imu.enable_stream(RS2_STREAM_GYRO, RS2_FORMAT_MOTION_XYZ32F, 200);

    m_pipe_imu.start(m_config_imu);
    m_imu_thread = std::thread(&RSLoader::imuThreadFunction, this);
}

RSLoader::~RSLoader()
{
    if (m_imu_thread.joinable())
    {
        m_imu_thread.join();
    }
    m_pipe_image.stop();
    m_pipe_imu.stop();
}

bool RSLoader::ShutDown()
{
    this->imu_switch = false;
    if (this->m_imu_thread.joinable())
    {
        // this->m_imu_thread.join();
        this->m_imu_thread.detach();
        if(this->record_switch){
            this->save_all_data();
            std::cout<<"save ok"<<std::endl;
        }

        std::cout << "realsense close return 11111" << std::endl;

        return true;
    }
    else
    {
        std::cout << "realsense close return 00000" << std::endl;
        return false;
    }
}
void RSLoader::initialize_files()
{
    // 获取当前时间
    std::time_t now = std::time(nullptr);
    std::tm *now_tm = std::localtime(&now);

    // 生成目录名，例如 "07151023"
    std::ostringstream dirname;
    dirname << std::setfill('0') << std::setw(2) << now_tm->tm_mon + 1
            << std::setw(2) << now_tm->tm_mday
            << std::setw(2) << now_tm->tm_hour
            << std::setw(2) << now_tm->tm_min;

    //    std::string base_path = "dataset/" + dirname.str();
    std::string base_path = dirname.str();

    this->base_path = this->base_path + base_path;
    // 创建目录
    std::string cmd = "mkdir -p " + this->base_path + "/cam0/data";
    int ret;
    ret = system(cmd.c_str());
    if(ret != 0){
        std::cerr << "[RSLoader]Command execution failed with return code: " << ret << std::endl;
    }

    cmd = "mkdir -p " + this->base_path + "/cam1/data";

    std::cout << std::endl << "cmd:" << cmd << std::endl;

    ret = system(cmd.c_str());
    if(ret != 0){
        std::cerr << "[RSLoader]Command execution failed with return code: " << ret << std::endl;
    }
    cmd = "mkdir -p " + this->base_path + "/imu0";
    ret = system(cmd.c_str());
    if(ret != 0){
        std::cerr << "[RSLoader]Command execution failed with return code: " << ret << std::endl;
    }

    // 初始化文件
    std::ofstream imu_file(this->base_path + "/imu0/data.csv", std::ios_base::trunc);
    imu_file << "#timestamp[s],w_RS_S_x[rads^-1],w_RS_S_y[rads^-1],w_RS_S_z[rads^-1],a_RS_S_x[ms^-2],a_RS_S_y[ms^-2],a_RS_S_z[ms^-2]\n";
    imu_file.close();

    std::ofstream cam0_file(this->base_path + "/cam0/data.csv", std::ios_base::trunc);
    cam0_file << "timestamp[s],filename\n";
    cam0_file.close();

    std::ofstream cam1_file(this->base_path + "/cam1/data.csv", std::ios_base::trunc);
    cam1_file << "timestamp[s],filename\n";
    cam1_file.close();
}

void RSLoader::save_images(const pair<double, Mat> &color, const pair<double, Mat> &depth)
{
    stringstream ss;
    ss << fixed << setprecision(6) << color.first;
    string timestamp = ss.str();
    // 保存彩色和深度图像
    string color_filename = this->base_path + "/cam0/data/" + timestamp + ".png";
    string depth_filename = this->base_path + "/cam1/data/" + timestamp + ".png";
    imwrite(color_filename, color.second);
    imwrite(depth_filename, depth.second);

    this->timestamp_pic.push_back(timestamp);
}

void RSLoader::save_all_data()
{
    // 保存IMU数据
    {
        string path = this->base_path + "/imu0/data.csv";
        ofstream imu_file(path, ios_base::app);
        int cnt1 = 0;
        imu_file << fixed << setprecision(6);
        for (const auto &imu_data : m_imu_save)
        {
            imu_file << imu_data[0] << ",";
            imu_file << imu_data[1] << ",";
            imu_file << imu_data[2] << ",";
            imu_file << imu_data[3] << ",";
            imu_file << imu_data[4] << ",";
            imu_file << imu_data[5] << ",";
            imu_file << imu_data[6] << endl;
            cnt1++;
        }
        std::cout << "number of imu" << cnt1 << std::endl;
    }

    {
        string path0 = this->base_path + "/cam0/data.csv";
        string path1 = this->base_path + "/cam1/data.csv";

        ofstream cam0_file(path0, ios_base::app);
        ofstream cam1_file(path1, ios_base::app);
        cam0_file << fixed << setprecision(6);
        cam1_file << fixed << setprecision(6);

        path0 = this->base_path + "/cam0/cam_timestamp.txt";
        path1 = this->base_path + "/cam1/cam_timestamp.txt";

        ofstream cam0_ts(path0, ios_base::app);
        ofstream cam1_ts(path1, ios_base::app);
        cam0_ts << fixed << setprecision(6);
        cam1_ts << fixed << setprecision(6);

        int cnt1 = 0;
        for (const auto &cam_data : this->timestamp_pic)
        {

            cam0_file << cam_data << "," << cam_data << ".png" << std::endl;
            cam1_file << cam_data << "," << cam_data << ".png" << std::endl;

            cam0_ts << cam_data << std::endl;
            cam1_ts << cam_data << std::endl;
            cnt1++;
        }
        std::cout << "number of pic" << cnt1 << std::endl;
    }
    std::cout << "save ok " << std::endl;
}

bool RSLoader::updateFrame(AppData &appData, FrameDataPtr frameDataPtr, SceneData &sceneData)
{

    if (appData.record && !this->record_switch)
    {
        this->record_switch=true;
        this->base_path = appData.offlineDataDir;

        initialize_files();
    }
    try
    {
        // 使用wait_for_frames等待新的帧数据
        // auto start = std::chrono::system_clock::now();
        rs2::frameset frames = m_pipe_image.wait_for_frames(); // wait for
        // auto end = std::chrono::system_clock::now();
        // std::chrono::duration<double>diff = end -start;
        // std::cout<<"waitforpipe TIME:  "<<diff.count()<<std::endl;

        rs2::align align_to_color(RS2_STREAM_COLOR);
        frames = align_to_color.process(frames);

        rs2::frame color_frame = frames.get_color_frame();
        rs2::frame depth_frame = frames.get_depth_frame();

        double ts = color_frame.get_timestamp() / 1e3;
        Mat color(Size(640, 480), CV_8UC3, (void *)color_frame.get_data(), Mat::AUTO_STEP);
        Mat depth(Size(640, 480), CV_16U, (void *)depth_frame.get_data(), Mat::AUTO_STEP);

        // std::cout << "----------------------------------[main] Frame: " << std::fixed << std::setprecision(6) << ts << std::endl;

        // convert depth map for viz
        cv::Mat depth_abs;
        cv::convertScaleAbs(depth, depth_abs, 0.03);  // 将深度图像转换为8位图像（即 cv::convertScaleAbs）
        cv::Mat depth_forViz;
        cv::applyColorMap(depth_abs, depth_forViz, cv::COLORMAP_JET);  // 使用 JET 颜色映射应用到深度图像上

        namedWindow("InputRealSense", WINDOW_AUTOSIZE);
        cv::imshow("InputRealSense-RGB", color);
        cv::imshow("InputRealSense-D", depth_forViz);
        cv::waitKey(1);
        frameDataPtr->imgColor = color.clone();
        frameDataPtr->imgDepth = depth.clone();
        frameDataPtr->timestamp = ts;

        // save image
        if (this->record_switch)
        {
            pair<double, Mat> colorsave;
            pair<double, Mat> depthsave;
            colorsave.first = ts;
            colorsave.second = color;
            depthsave.first = ts;
            depthsave.second = depth;
            save_images(colorsave, depthsave);
        }

        auto it = this->m_imu_all.begin();
        while (true)
        {
            std::lock_guard<std::mutex> lock(m_imu_mutex);

            if (it != this->m_imu_all.end())
            {
                if ((*it)[0] < ts)
                {
                    // std::cout << "[main] IMU: " << std::fixed << std::setprecision(6) << (*it)[0] << std::endl;
                    sceneData.imuBuffer.push_back(*it);

                    it = this->m_imu_all.erase(it);
                }
                else
                {
                    break;
                }
            }
            else
            {
                std::cout << "[main] Waiting for enough imu data." << std::endl;
                break;
            }
        }
        return true;
    }
    catch (const rs2::error &e)
    {
        std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "): " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception &e)
    {
        std::cout << "error detected" << std::endl;
        std::cerr << e.what() << std::endl;
        return false;
    }
}


void RSLoader::imuThreadFunction(){

    double old_ts=-1;
    double ts;
    vector<double> imu_data_old(7, 0);
    vector<double> imu_data_new(7, 0);

    while (this->imu_switch)
    {
        rs2::frameset imu_frames;
        if (m_pipe_imu.poll_for_frames(&imu_frames))
        {
            // lock_guard<mutex> lock(m_imu_mutex);
            rs2::motion_frame accel_frame = imu_frames.first_or_default(RS2_STREAM_ACCEL);
            rs2::motion_frame gyro_frame = imu_frames.first_or_default(RS2_STREAM_GYRO);

            if (accel_frame && gyro_frame)
            {
                ts = accel_frame.get_timestamp() / 1e3;
                imu_data_new[0] = ts;
                rs2_vector accel_sample = accel_frame.get_motion_data();
                imu_data_new[4] = accel_sample.x;
                imu_data_new[5] = accel_sample.y;
                imu_data_new[6] = accel_sample.z;
                rs2_vector gyro_sample = gyro_frame.get_motion_data();
                imu_data_new[1] = gyro_sample.x;
                imu_data_new[2] = gyro_sample.y;
                imu_data_new[3] = gyro_sample.z;

                // std::cout << "Accel:" << accel_sample.x << ", " << accel_sample.y << ", " << accel_sample.z << std::endl;
                // std::cout << "Gyro:" << gyro_sample.x << ", " << gyro_sample.y << ", " << gyro_sample.z << std::endl;
                //                std::cout<<std::fixed<<std::setprecision(6)<<ts<<std::endl;
                if (ts != old_ts && old_ts != -1)
                {
                    // std::lock_guard<std::mutex> lock(sceneData.imu_mutex);
                    std::lock_guard<std::mutex> lock(this->m_imu_mutex);
                    this->m_imu_all.push_back(imu_data_old);

                    if(this->record_switch){
                        this->m_imu_save.push_back(imu_data_old);
                    }

                    //                    std::cout<<"imu push ok"<<std::endl;
                }
                imu_data_old = imu_data_new;
                old_ts = ts;
            }
        }
    }
}
