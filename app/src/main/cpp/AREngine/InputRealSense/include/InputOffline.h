#pragma once

#include"Basic/include/ARModule.h"
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

class OfflineLoader;

class InputOffline : public ARModule {
    std::shared_ptr<OfflineLoader> _impl;

public:
    InputOffline();

    virtual int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override;
    virtual int Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override;
    virtual int ShutDown(AppData& appData, SceneData& sceneData) override;
};