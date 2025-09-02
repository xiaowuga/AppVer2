#include <iostream>
#include <iomanip>
#include <fstream>

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include"Detection/include/SegAndPoseEstimation.h"

using namespace std;
using namespace cv; 


//mutex imu_mutex;

RSLoader::RSLoader(int width, int height, int fps){

}

RSLoader::~RSLoader(){
}

bool RSLoader::updateFrame(SceneData &sceneData, FrameData &frameData){
   
   // store at sceneData.get

   return true;
}
