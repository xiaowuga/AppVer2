//
// Created by xiaow on 2025/9/13.
//

#include "PoseEstimationRokid.h"
#include "demos/cube.h"
#include <openxr/openxr.h>
#include <common/xr_linear.h>
#include <glm/gtc/type_ptr.hpp>

RokidHandPose* RokidHandPose::instance() {
    static RokidHandPose *ptr = nullptr;
    if(!ptr) {
        ptr = new RokidHandPose();
        ptr->hand_pose.resize(2);
        ptr->hand_pose[0].tag = hand_tag::Left;
        ptr->hand_pose[1].tag = hand_tag::Right;
    }
    return ptr;
}

void RokidHandPose::set(const XrHandJointLocationEXT *location) {
    std::unique_lock<std::shared_mutex> _lock(_dataMutex);
    joint_loc.clear();
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
                joint_loc.emplace_back(mat);

            }
        }
    }
}

std::vector<HandPose> &RokidHandPose::get_hand_pose() {
    return hand_pose;
}

std::vector<glm::mat4>& RokidHandPose::get_joint_loc() {
    return joint_loc;
}


int PoseEstimationRokid::Init(AppData &appData,SceneData &sceneData,FrameDataPtr frameDataPtr){
    hand_pose.resize(2);
    hand_pose[0].tag = hand_tag::Left;
    hand_pose[1].tag = hand_tag::Right;
    return STATE_OK;
}

int PoseEstimationRokid::Update(AppData &appData,SceneData &sceneData,FrameDataPtr frameDataPtr){

    std::vector<HandPose>& hand_pose = RokidHandPose::instance()->get_hand_pose();
    glm::mat4 relocMatrix = frameDataPtr->jointRelocMatrix;

    for(int i = 0; i < hand_pose.size(); i++) {
        auto& joint = hand_pose[i].joints;
        for(int j = 0; j < joint.size(); j++) {
            glm::vec4 pos(joint[j][0], joint[j][1], joint[j][2], 1.0);
            pos = relocMatrix * pos;
            joint[j] = cv::Vec3f(pos.x, pos.y, pos.z);
        }
    }

    frameDataPtr->handPoses = hand_pose;

    joint_loc = RokidHandPose::instance()->get_joint_loc();

    for(int i = 0;  i < joint_loc.size(); i++) {
        joint_loc[i] = relocMatrix * joint_loc[i];
    }

    return STATE_OK;
}

int PoseEstimationRokid::CollectRemoteProcs(SerilizedFrame &serilizedFrame,std::vector<RemoteProcPtr> &procs,FrameDataPtr frameDataPtr){

    return STATE_OK;
}


int PoseEstimationRokid::ProRemoteReturn(RemoteProcPtr proc) {

    return STATE_OK;
}

int PoseEstimationRokid::ShutDown(AppData &appData,SceneData &sceneData){
    return STATE_OK;
}


std::vector<glm::mat4>& PoseEstimationRokid::get_joint_loc() {
    return joint_loc;
}
