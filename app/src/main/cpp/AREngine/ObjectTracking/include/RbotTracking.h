//
// Created by zhang on 23-9-15.
//
#ifndef ARENGINE_RBOTTRACKING_H
#define ARENGINE_RBOTTRACKING_H

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "Basic/include/ARModule.h"
#include "Basic/include/BasicData.h"
// #include "RBOT/src/object3d.h"
// #include "RBOT/src/pose_estimator6d.h"
// #include "../asset/RBOT/src/object3d.h"
// #include "../asset/RBOT/src/pose_estimator6d.h"
#include "ObjectTracking/asset/RBOT/src/object3d.h"
#include "ObjectTracking/asset/RBOT/src/pose_estimator6d.h"


class RbotTracking{

public:
    int width;
    int height;

    // near and far plane of the OpenGL view frustum
    float zNear=10.0;
    float zFar=10000.0;

    int timeout;

    bool showHelp;

    int vfr{1}; // set 1 to activate VFR

    // camera instrinsics
    cv::Matx33f K;
    cv::Matx14f distCoeffs;

    std::string shader_path;
    std::string data_path;

    // distances for the pose detection template generation
    std::vector<float> distances={200.0f, 400.0f, 600.0f};

    // load 3D objects
    std::vector<Object3D*> objects;
    std::vector<Object3D*> realObjects;
    std::vector<Object3D*> virtualObjects;

    // create the pose estimator
    PoseEstimator6D* poseEstimator;

    RenderingEngine* renderingEngine;

public:

    RbotTracking();

    ~RbotTracking();

    void PreCompute(std::string configPath);

    void Init(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr);

    void Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr);

    cv::Mat DrawResultOverlay(const cv::Mat& frame);


};

#endif //ARENGINE_RBOTTRACKING_H