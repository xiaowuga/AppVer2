//
// Created by Zhang Yidi on 2023/9/14.
//

#ifndef ARENGINE_AlignModule_H
#define ARENGINE_AlignModule_H
#include "Basic/include/opencv3_definition.h"
#include "opencv2/opencv.hpp"
#include "Basic/include/ARModule.h"
#include "Basic/include/BasicData.h"
#include "Basic/include/ConfigLoader.h"

#include <memory>

#include "rvgVIOSystem.h"

/**
 * AlignModule：相机跟踪类，继承于算法基类
 * 功能：对每一帧相机进行跟踪
 * 输入：RGBD图像及IMU
 * 输出：每帧相机的位姿，存放到SceneData::cameraPose
 */

class AlignModule: public ARModule {
public:
     AlignModule();
     ~AlignModule() override;

    void PreCompute(std::string configPath) override;

    int Init(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) override;

    int Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) override;
    
    int CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) override;

    int ProRemoteReturn(RemoteProcPtr proc) override;
    
};

// #include "AlignModule.cpp"
#endif //ARENGINE_AlignModule_H
