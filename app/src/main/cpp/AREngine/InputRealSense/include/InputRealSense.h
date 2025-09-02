#pragma once

#include"Basic/include/ARModule.h"
class RSLoader;

class InputRealSense : public ARModule {
    std::shared_ptr<RSLoader> _impl;

public:
    InputRealSense();

    virtual int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override;
    virtual int Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override;
    virtual int ShutDown(AppData& appData, SceneData& sceneData) override;

};

