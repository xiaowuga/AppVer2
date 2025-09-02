#pragma once

#include "BasicData.h"
#include "ARModule.h"

// #include "Basic/include/ConfigLoader.h"
#include "App.h"

class PoseEstimationFetch : public ARModule {
public:
    PoseEstimationFetch() = default;
    ~PoseEstimationFetch() = default;

    int Init(AppData& appData, SceneData& SceneData, FrameDataPtr frameDataPtr) override;
	int Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override;
	int ShutDown(AppData& appData, SceneData& sceneData) override;
    int CollectRemoteProcs(
        SerilizedFrame &serilizedFrame,
        std::vector<RemoteProcPtr> &procs,
        FrameDataPtr frameDataPtr) override;
    int ProRemoteReturn(RemoteProcPtr proc) override;
public:

    bool handFlag;
    // 长度126=2*21*3，表示双手21个关节点的三维姿态
    // 其中0~62是属于左手的关节点信息，63~125是属于右手的关节点信息
    std::vector<float> handPosition;
    // 关节点的MANO旋转信息，目前没有用到
    std::vector<float> rotationResult;
    // 关节点的的MANO形态信息，目前没有用到
    std::vector<float> shapeResult;
    // 关节点在二维图像上的投影位置
    std::vector<float> joints2dResult;
};