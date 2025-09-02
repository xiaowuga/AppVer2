#include "InteractionInputOffline.h"

InteractionInputOffline::InteractionInputOffline(){
    _moduleName = "InteractionInputOffline";
}

InteractionInputOffline::~InteractionInputOffline(){
    cap.release();
}

int InteractionInputOffline::Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameData){
    if (!cap.open(appData.offlineDataDir + "V0.avi")){
        std::cerr << "Error opening video stream or file" << std::endl;
        return STATE_ERROR;
    }
    return STATE_OK;
}

int InteractionInputOffline::Update(AppData &appData, SceneData& sceneData, FrameDataPtr frameData){
    cap >> frameData->imgColor;
    if (frameData->imgColor.empty()) {
        cap.set(cv::CAP_PROP_POS_FRAMES, 0);
        cap >> frameData->imgColor;
    }
    cv::resize(frameData->imgColor, frameData->imgColor, cv::Size(640, 480));
    cv::Mat depthImage = cv::Mat::zeros(640, 480, CV_16UC1);
    depthImage.setTo(255.0);
    frameData->imgDepth = depthImage;
    frameData->image.push_back(frameData->imgColor);
    frameData->depth.push_back(frameData->imgDepth);
    frameData->timestamp = 0;
    //std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return STATE_OK;
}