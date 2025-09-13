#ifndef ARENGING_HEADUNDER_H
#define ARENGING_HEADUNDER_H

#include "ARModule.h"
#include "BasicData.h"
/**
 * 功能：对头部骨架进行理解
 * 输入：头部骨架序列
 * 输出：
*/
class HeadUnderstanding : public ARModule{
    public:
        HeadUnderstanding();
        ~HeadUnderstanding() override;
        void PreCompute(std::string configPath) override;
        int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameData) override;
        int Update(AppData &appData, SceneData& sceneData, FrameDataPtr frameData) override;
        int ShutDown(AppData& appData,  SceneData& sceneData) override;
        int CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) override;
};

#endif