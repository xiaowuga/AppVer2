#pragma once
#ifndef ARENGING_INTERACTIONINPUTOFFLIE_H
#define ARENGINE_INTERACTIONINPUTOFFLIE_H

#include "ARModule.h"
#include <thread>

class InteractionInputOffline : public ARModule {
public:
    InteractionInputOffline();
    ~InteractionInputOffline() override;
    int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameData) override;
    int Update(AppData &appData, SceneData& sceneData, FrameDataPtr frameData) override;
private:
    cv::VideoCapture cap;

};

#endif //ARENGINE_INTERACTIONINPUTOFFLIE_H