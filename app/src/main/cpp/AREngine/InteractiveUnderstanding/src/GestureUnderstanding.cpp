#include "GestureUnderstanding.h"


GestureUnderstanding::GestureUnderstanding() {
    _moduleName = "GestureUnderstanding";
};
GestureUnderstanding::~GestureUnderstanding(){

};

void GestureUnderstanding::PreCompute(std::string configPath) {

};
int GestureUnderstanding::Init(AppData &appData, SceneData &sceneData, FrameDataPtr frameData) {
    //初始化手关节点模型列表
    _handNodes.clear();
    //获取相机位置
    //向sceneData中添加预制手关节点模型
    for (int i = 0; i < 42; i++) {
        SceneObjectPtr handNode(new VirtualObject());
		handNode->name = "handNode_" + std::to_string(i);
        handNode->filePath = appData.dataDir+"/handNode/" + handNode->name + ".fb";
        handNode->Init();
        _handNodes.push_back(handNode);
        sceneData.setObject(handNode->name, handNode);
    }
    return STATE_OK;

};
int GestureUnderstanding::Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameData) {
    
    //更新手部骨架位置
    frameData->gestureDataPtr = std::make_shared<GestureUnderstandingData>();
    if(frameData->handPoses.size() != 2){
		std::cerr << "HandPose size in frameData less than 2" << std::endl;
        return STATE_ERROR;
    }
    else {
        //更新手势类别
        frameData->gestureDataPtr->curLGesture = _gesturePredictor.predict(frameData->handPoses[0]);
        frameData->gestureDataPtr->curRGesture = _gesturePredictor.predict(frameData->handPoses[1]);
        //更新手关节点模型的位置
		// TODO: 统一坐标系
        //更新相机变化矩阵
        SceneObject* cam = sceneData.getMainCamera();
        int i = 0;
        for (HandPose handPose : frameData->handPoses) {
            for (cv::Vec3f joint : handPose.getjoints()) {
				cv::Matx44f jointMat = cv::Matx44f::eye();
                joint *= 1.f;
                // World xyz <---> PoseEsitimation xyz
				jointMat(0, 3) = joint[0];
				jointMat(1, 3) = joint[1];
				jointMat(2, 3) = joint[2];
				// Align with the camera
				cv::Matx44f camInitRot = cam->initTransform.GetRotation().inv();
				cv::Matx44f camInitPos = camInitRot * cam->initTransform.GetMatrix();
				camInitPos(0, 3) = -camInitPos(0, 3);
				camInitPos(1, 3) = -camInitPos(1, 3);
				camInitPos(2, 3) = -camInitPos(2, 3);
				cv::Matx44f initTransForm = camInitPos * camInitRot * jointMat * _initScale;
				_handNodes[i]->initTransform.setPose(initTransForm);

				cv::Matx44f camRot = cam->transform.GetRotation().inv();
				cv::Matx44f camPos = camRot * cam->transform.GetMatrix();
				camPos(0, 3) = -camPos(0, 3);
				camPos(1, 3) = -camPos(1, 3);
				camPos(2, 3) = -camPos(2, 3);
                cv::Matx44f transForm = camPos * camRot;
                _handNodes[i]->transform.setPose(transForm);
                i++;
            }
        }
    }
    return STATE_OK;
};
int GestureUnderstanding::ShutDown(AppData &appData, SceneData &sceneData) {
    return STATE_OK;
};
int GestureUnderstanding::CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) {
    return 0;
};
int GestureUnderstanding::ProRemoteReturn(RemoteProcPtr proc) {
    return 0;
};
