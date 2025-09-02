//#include"Basic/include/PoseUtils.h"
#include "PoseUtils.h"

cv::Matx44f convertMatToMatx44f(const cv::Mat& mat) {
    // 检查 Mat 的大小是否为 4x4 且类型是否为 CV_32F
    if (mat.rows == 4 && mat.cols == 4 && mat.type() == CV_32F) {
        // 创建一个 Matx44f 并将 Mat 的数据复制进去
        cv::Matx44f matx;
        mat.copyTo(cv::Mat(4, 4, CV_32F, matx.val));  // 复制数据到 Matx44f 的存储
        return matx;
    } else {
        // 如果尺寸或类型不匹配，返回一个默认空ni sheni的 Matx44f
        std::cerr << "Error: Input matrix must be 4x4 and of type CV_32F." << std::endl;
        std::cerr << "Current matrix size: " << mat.rows << "x" << mat.cols << std::endl;
        std::cerr << "Current matrix type: " << mat.type() << " (Expected CV_32F: " << CV_32F << ")" << std::endl;
        return cv::Matx44f::zeros();  // 返回一个全零的 Matx44f
    }
}
