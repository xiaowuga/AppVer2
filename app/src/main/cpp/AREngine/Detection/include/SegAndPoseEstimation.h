#include <iostream>
#include <iomanip>
#include <fstream>

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include "BasicData.h"


using namespace std;
using namespace cv;

class SegAndPosEstimation{
    public:
        SegAndPosEstimation(int cat_id=1); // add param you need
        ~SegAndPosEstimation();

        getMaskAndPose(cv::Mat &mask, cv::Mat44f &Pose);
};