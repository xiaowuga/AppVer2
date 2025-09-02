//
// 
//

#ifndef RELICALIZATION2_H
#define RELICALIZATION2_H

#include <opencv2/opencv.hpp>
#include <memory>
#include "opencv3_definition.h"
#include <mutex>

namespace Reloc {

class Relocalization_;

class Relocalization {
public:
    bool Init(std::string& config_file, std::string& vocabulary_file, std::string map_file, bool is_loadmap, bool is_save_map);

    void processRGBD_Pose(double timestamp, cv::Mat& frame, cv::Mat& depth_image, cv::Mat &pose);

    bool alignSLAM2Reloc(cv::Mat& alignTransform);

    void Shutdown();

    std::vector<cv::Mat> getPoses();

private:
    std::shared_ptr<Relocalization_> sys;
    std::mutex mtx_;
};
}


#endif //RELICALIZATION2_H
