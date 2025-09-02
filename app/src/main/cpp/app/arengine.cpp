#include"arengine.h"
using namespace cv;

ARInputSources* ARInputSources::instance() {
    static ARInputSources *ptr=nullptr;
    if(!ptr)
        ptr=new ARInputSources();
    return ptr;
}

void ARInputSources::set(const ARInputSources::FrameData &frameData, int mask) {
    std::unique_lock<std::shared_mutex> _lock(_dataMutex);
    if(mask&DATAF_IMAGE)        _frameData.img=frameData.img;
    if(mask&DATAF_TIMESTAMP)    _frameData.timestamp=frameData.timestamp;
    if(mask&DATAF_CAMERAMAT)    _frameData.cameraMat=frameData.cameraMat;
}

void ARInputSources::get(ARInputSources::FrameData &frameData, int mask) {
    std::shared_lock<std::shared_mutex> _lock(_dataMutex);
    frameData=_frameData;
}

int ARInputs::Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr)
{
    ARInputSources::FrameData frameData;
    ARInputSources::instance()->get(frameData);

    while(frameData.timestamp==this->_lastTimestamp)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        ARInputSources::instance()->get(frameData);
    }
    _lastTimestamp=frameData.timestamp;

    if(!frameData.img.empty())
    {
        cv::Mat img=frameData.img, dst;

        if(_udistMap1.empty())
        {
            cv::Matx33f K = { 281.60213015, 0.,  318.69481832,  0., 281.37377039, 243.6907021, 0., 0., 1. };
            cv::Mat distCoeffs = (cv::Mat_<float>(1, 4) << 0.11946399, 0.06202764, -0.28880297, 0.21420146);

            Mat newK;
            cv::fisheye::estimateNewCameraMatrixForUndistortRectify(K, distCoeffs, img.size(), noArray(), newK, 0.f);

            cv::fisheye::initUndistortRectifyMap(K, distCoeffs, noArray(), newK, img.size(), CV_16SC2, _udistMap1, _udistMap2);
            _camK=newK;
        }

        cv::remap(img, dst, _udistMap1, _udistMap2, INTER_LINEAR);
        img=dst;

        frameDataPtr->image.push_back(img);
        frameDataPtr->colorCameraMatrix=_camK;
    }

    sceneData.setData("ARInputs", frameData);
    return STATE_OK;
}


