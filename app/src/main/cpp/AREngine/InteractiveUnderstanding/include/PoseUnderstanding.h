#ifndef ARENGING_POSEUNDER_H
#define ARENGING_POSEUNDER_H


#include "ARModule.h"
#include "BasicData.h"
/**
 * 功能：对人体位姿进行理解
 * 输入：人体位姿骨架序列
 * 输出：
*/
class PoseUnderstanding : public ARModule{
    public:
        PoseUnderstanding();
        ~PoseUnderstanding() override;
        void PreCompute(std::string configPath) override;
        int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameData) override;
        int Update(AppData &appData, SceneData& sceneData, FrameDataPtr frameData) override;
        int ShutDown(AppData& appData,  SceneData& sceneData) override;
        int CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) override;

};
#endif