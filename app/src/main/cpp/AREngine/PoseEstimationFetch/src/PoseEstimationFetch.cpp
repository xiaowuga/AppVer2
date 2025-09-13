#include "PoseEstimationFetch.h"
#include <chrono>
#include <thread>

int PoseEstimationFetch::Init(AppData& appData, SceneData& SceneData, FrameDataPtr frameDataPtr) {
    handFlag = false;
    return STATE_OK;
}

int PoseEstimationFetch::Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // 使用FetchPose从远程获取姿态信息
//    SerilizedObjs cmdSend = {
//        {"cmd", std::string("FetchPose")}
//    };
//    app->postRemoteCall(this, nullptr, cmdSend);

    //上传手势数据到FrameData
    if(handFlag) {
        std::vector<HandPose> handPoses(2);
        std::array<cv::Vec3f, 21> left_joint_positions;
        std::array<cv::Vec3f, 21> right_joint_positions;
        std::array<std::array<float, 2>, 21> joints2d;
        std::array<std::array<std::array<float, 3>, 3>, 16> rotation_matrices;
        std::array<float, 10> shape_params;
        cv::Vec3f origin;

        for (int i = 0; i < 63; i += 3) {
            cv::Vec3f item(handPosition[i], handPosition[i + 1], handPosition[i + 2]);
            left_joint_positions[i / 3] = item;
        }
        for (int i = 63; i < 126; i += 3) {
            cv::Vec3f item(handPosition[i], handPosition[i + 1], handPosition[i + 2]);
            right_joint_positions[(i - 63) / 2] = item;
        }
        handPoses[0] = HandPose(static_cast<int>(hand_tag::Left), left_joint_positions,
                                joints2d, rotation_matrices, shape_params, origin);
        handPoses[1] = HandPose(static_cast<int>(hand_tag::Right), left_joint_positions,
                                joints2d, rotation_matrices, shape_params, origin);

        frameDataPtr->handPoses = handPoses;
    }
    return STATE_OK;
}

int PoseEstimationFetch::ShutDown(AppData& appData, SceneData& sceneData) {
    return STATE_OK;
}

int PoseEstimationFetch::CollectRemoteProcs(
    SerilizedFrame &serilizedFrame,
    std::vector<RemoteProcPtr> &procs,
    FrameDataPtr frameDataPtr
) {

    SerilizedObjs cmdSend = {
            {"cmd", std::string("FetchPose")}
    };
//    app->postRemoteCall(this, nullptr, cmdSend);

    procs.push_back(std::make_shared<RemoteProc>(this,frameDataPtr,cmdSend,
                                                 RPCF_SKIP_BUFFERED_FRAMES));
    return STATE_OK;
}

int PoseEstimationFetch::ProRemoteReturn(RemoteProcPtr proc) {
    // 从远程获得姿态数据
    std::cout << "Get pose from remote" << std::endl;
    auto& send = proc->send;
    auto& ret = proc->ret;
    auto cmd = send.getd<std::string>("cmd");

    if (cmd == "FetchPose") {
        // 标识手部姿态是否有效
        handFlag = ret.getd<bool>("handPose_OK");
        // 长度126=2*21*3，表示双手21个关节点的三维姿态
        // 其中0~62是属于左手的关节点信息，63~125是属于右手的关节点信息
        handPosition= ret.getd<std::vector<float>>("handPosition");
//        // 关节点的MANO旋转信息，目前没有用到
//        rotationResult = ret.getd<std::vector<float>>("handPose");
//        // 关节点的的MANO形态信息，目前没有用到
//        shapeResult = ret.getd<std::vector<float>>("handShape");
//        // 关节点在二维图像上的投影位置
//        joints2dResult = ret.getd<std::vector<float>>("joints_img");
//        std::cout << "handFlag: " << handFlag << std::endl;
    }

    return STATE_OK;
}