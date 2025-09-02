
#include <Eigen/Eigen>
#include <opencv2/core/eigen.hpp>
#include <unistd.h> 


std::string get_tum_string(double pose_timestamp, const cv::Mat& pose_)
{
    // std::cout << "get_tum_string..." << std::endl;
    // transform the pose matrix to CV_32F
    cv::Mat pose;
    // std::cout << "pose_.type():" << pose_.type() << std::endl;
    if (pose_.type() != CV_32F)
    {
        // std::cout << "pose_.type():" << pose_.type() << std::endl;
        pose_.convertTo(pose, CV_32F);
        // std::cout << "convert pose to CV_32F" << std::endl;
    }
    else
    {
        pose = pose_;
    }

    std::stringstream ss;

    ss << std::fixed;
    ss << std::setprecision(6);
    // timestamp
    ss << pose_timestamp << " ";
    // translation
    ss << pose.at<float>(0, 3) << " " << pose.at<float>(1, 3) << " " << pose.at<float>(2, 3) << " ";
    // quaternion
    Eigen::Matrix3f R;
    cv::cv2eigen(pose(cv::Rect(0, 0, 3, 3)), R);
    Eigen::Quaternionf q(R);
    ss << q.x() << " " << q.y() << " " << q.z() << " " << q.w() << std::endl;
    // Convert the stringstream to a string and return
    std::string ret = ss.str();
    return ret;
}

void save_pose_as_tum(std::string output_file_path, double pose_timestamp, cv::Mat pose)
{
    std::string output_dir = output_file_path.substr(0, output_file_path.find_last_of("/"));

    if (access(output_dir.c_str(), 0) == -1)
    {
        std::string cmd = "mkdir -p " + output_dir;
        int ret = system(cmd.c_str());
        if(ret != 0){
            std::cerr << "[save_pose_as_tum]Command execution failed with return code: " << ret << std::endl;
        }
    }
    std::ofstream out_file(output_file_path, std::ios::out|std::ios::app);
    std::string ret = get_tum_string(pose_timestamp, pose);
    // std::cout << "get tum string: " << ret << std::endl;
    out_file << ret;
}