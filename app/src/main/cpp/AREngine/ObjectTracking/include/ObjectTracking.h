#ifndef ARENGINE_OBJECTTRACKING_H
#define ARENGINE_OBJECTTRACKING_H

#include <opencv2/opencv.hpp>
#include "Basic/include/ARModule.h"
#include "Basic/include/BasicData.h"
#include "Basic/include/ConfigLoader.h"

class RbotTracking;
/**
 * 功能：对已知模型的物体进行三维跟踪
 * 输入：视频帧及物体三维模型。物体三维模型在应用开发引擎中定义，并通过PreCompute函数进行预处理
 * 输出：每个物体的三维位姿，存放到SceneData::objectPose
 */

class ObjectTracking : public ARModule {
private:

    RbotTracking* tracker;

public:

    ObjectTracking();

    ~ObjectTracking() override;

    void PreCompute(std::string configPath) override;

    int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override;

    int Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override;

    int ShutDown(AppData& appData, SceneData& sceneData) override;

    int CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) override;

    int ProRemoteReturn(RemoteProcPtr proc) override;

};


#endif //ARENGINE_OBJECTTRACKING_H
