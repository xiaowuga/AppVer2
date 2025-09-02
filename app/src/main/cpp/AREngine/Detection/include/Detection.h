#pragma once


#include"Basic/include/ARModule.h"

class RSLoader;

class InputRealSense
	:public ARModule
{
	std::shared_ptr<RSLoader> _impl;
public:
    virtual int Init(AppData& appData, FrameData& frameData, SceneData& sceneData);

    virtual int Update(AppData& appData, FrameData& frameData, SceneData& sceneData);
};

