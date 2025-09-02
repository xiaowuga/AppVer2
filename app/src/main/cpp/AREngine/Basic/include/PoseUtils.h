#include <opencv2/opencv.hpp>
#include <string>
#include <fstream>

cv::Matx44f convertMatToMatx44f(const cv::Mat& mat);

// void save_pose_as_tum(std::string output_file_path, double pose_timestamp, cv::Mat pose);