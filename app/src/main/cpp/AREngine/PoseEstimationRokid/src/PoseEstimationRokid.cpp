//
// Created by xiaow on 2025/9/13.
//

#include "PoseEstimationRokid.h"
#include "demos/cube.h"
#include <openxr/openxr.h>
#include <common/xr_linear.h>
#include <glm/gtc/type_ptr.hpp>


int PoseEstimationRokid::Init(AppData &appData,SceneData &sceneData,FrameDataPtr frameDataPtr){
    hand_OK = false;
    hand_pose.resize(2);
    hand_pose[0].tag = hand_tag::Left;
    hand_pose[1].tag = hand_tag::Right;
    return STATE_OK;
}

int PoseEstimationRokid::Update(AppData &appData,SceneData &sceneData,FrameDataPtr frameDataPtr){

    frameDataPtr->handPoses = hand_pose;

    return STATE_OK;
}

int PoseEstimationRokid::CollectRemoteProcs(SerilizedFrame &serilizedFrame,std::vector<RemoteProcPtr> &procs,FrameDataPtr frameDataPtr){
    SerilizedObjs cmdSend = {
            {"cmd", std::string("PoseEstimationRokid")}
    };

    procs.push_back(std::make_shared<RemoteProc>(this,frameDataPtr,cmdSend,
                                                 RPCF_SKIP_BUFFERED_FRAMES)); //添加add命令，将输入帧的像素值加上指定的值，并返回结果图像(参考TestServer.cpp中TestPro1Server类的实现)

    return STATE_OK;
}

int PoseEstimationRokid::ProRemoteReturn(RemoteProcPtr proc) {
    auto &send=proc->send;
    auto &ret=proc->ret;
    auto cmd=send.getd<std::string>("cmd");


    return STATE_OK;
}

int PoseEstimationRokid::ShutDown(AppData &appData,SceneData &sceneData){
    return STATE_OK;
}

void PoseEstimationRokid::setHandJointLocation(const XrHandJointLocationEXT* location) {
    std::vector<glm::mat4> tmp;
    for (auto hand = 0; hand < HAND_COUNT; hand++) {
        for (int i = 0; i < XR_HAND_JOINT_COUNT_EXT; i++) {
            int index = hand * XR_HAND_JOINT_COUNT_EXT + i;
            const XrHandJointLocationEXT& jointLocation = location[index];
            if (jointLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT && jointLocation.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT) {

                int idx = mapping[i];
                if(idx == -1)
                    continue;
                XrMatrix4x4f m{};
                XrVector3f scale{1.0f, 1.0f, 1.0f};
                XrMatrix4x4f_CreateTranslationRotationScale(&m, &jointLocation.pose.position, &jointLocation.pose.orientation, &scale);

                glm::mat4 mat = glm::make_mat4((float*)&m);
                hand_pose[hand].joints[i] = cv::Vec3f(jointLocation.pose.position.x, jointLocation.pose.position.y, jointLocation.pose.position.z);
                tmp.emplace_back(mat);

            }
        }
    }

    joint_loc = tmp;

    hand_OK = true;
}