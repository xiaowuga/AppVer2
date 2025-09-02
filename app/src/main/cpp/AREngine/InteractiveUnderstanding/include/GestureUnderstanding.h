#ifndef ARENGING_GESTUREUNDER_H
#define ARENGING_GESTUREUNDER_H

//#include "Basic/include/ARModule.h"
//#include "Basic/include/BasicData.h"
//#include "Basic/include/ConfigLoader.h"
//#include "Basic/include/App.h"
#include "ARModule.h"
#include "BasicData.h"
#include "ConfigLoader.h"
#include "App.h"
#include "GesturePredictor.h"

/**
 * 功能：对手部骨架进行理解
 * 输入：手部骨架序列
 * 输出：当前帧手势类别，存储于FrameData中
*/
class GestureUnderstanding: public ARModule{
private:

    /**
     * 手关节点模型
     */
    std::vector<SceneObjectPtr> _handNodes;
    /**
     * 手关节点位置
    */
    std::vector<HandPose> _handPoses;
    /**
     * 手势类别预测器
    */
    GesturePredictor _gesturePredictor;
    /*
    * 初始关节点偏移
    */
	cv::Matx44f _initOffset = cv::Matx44f(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
        );

    /*
	* 手部关节点模型缩放比例
    */
	cv::Matx44f _initScale = cv::Matx44f(
		0.001, 0, 0, 0,
		0, 0.001, 0, 0,
		0, 0, 0.001, 0,
		0, 0, 0, 1
	);
public:
    GestureUnderstanding();
    ~GestureUnderstanding() override;
    void PreCompute(std::string configPath) override;
    int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameData) override;
    int Update(AppData &appData, SceneData& sceneData, FrameDataPtr frameData) override;
    int ShutDown(AppData& appData,  SceneData& sceneData) override;
    int CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) override;
    int ProRemoteReturn(RemoteProcPtr proc) override;
};
#endif
