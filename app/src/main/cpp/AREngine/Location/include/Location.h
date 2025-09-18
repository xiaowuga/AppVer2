//
// Created by xiaow on 2025/9/7.
//

#ifndef ROKIDOPENXRANDROIDDEMO_LOCATION_H
#define ROKIDOPENXRANDROIDDEMO_LOCATION_H

#include "BasicData.h"
#include "ARModule.h"
#include "arucopp.h"
#include "glm/glm.hpp"
#include "ARInput.h"
#include "App.h"

class ProjectMatrixSource {
public:
    static ProjectMatrixSource* instance();


    void set(const glm::mat4& projectMatrix);

    const glm::mat4& get() const;
private:
    glm::mat4 _projectMatrix;
    std::shared_mutex  _dataMutex;
};


class Location : public ARModule {
public:
    ArucoPP  _detector;
    glm::mat4 markerPoseInCamera;
    glm::mat4 sensorPose_inv;
    glm::mat4 computedCameraPose;
    glm::mat4 view;
    std::shared_mutex  _dataMutex;

public:

    int Init(AppData& appData, SceneData& SceneData, FrameDataPtr frameDataPtr) override;
    int Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) override;
    int ShutDown(AppData& appData, SceneData& sceneData) override;
    int CollectRemoteProcs(
            SerilizedFrame &serilizedFrame,
            std::vector<RemoteProcPtr> &procs,
            FrameDataPtr frameDataPtr) override;
    int ProRemoteReturn(RemoteProcPtr proc) override;



};


#endif //ROKIDOPENXRANDROIDDEMO_LOCATION_H
