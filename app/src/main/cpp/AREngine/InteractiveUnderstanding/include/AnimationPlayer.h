//
// Created by 21718 on 2025/8/31.
//

#ifndef ROKIDOPENXRANDROIDDEMO_ANIMATIONPLAYER_H
#define ROKIDOPENXRANDROIDDEMO_ANIMATIONPLAYER_H

#include "ARModule.h"
#include "Animator.h"
#include "json.hpp"
#include <fstream>

// AnimationPlayer is used to call playNextFrame of all animators in Update()
class AnimationPlayer : public ARModule {
public:
    AnimationPlayer() = default;
    ~AnimationPlayer() override = default;
    void PreCompute(std::string configPath) override {}
    int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameData) override;
    int Update(AppData &appData, SceneData& sceneData, FrameDataPtr frameData) override;
    int ShutDown(AppData& appData,  SceneData& sceneData) override { return STATE_OK; }
    int CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) override { return STATE_OK; }
    int ProRemoteReturn(RemoteProcPtr proc) override { return STATE_OK; }

    void addAnimator(std::string object_name, std::shared_ptr<Animator::Animator> animator) {
        animators.emplace(object_name, animator);
    }
    std::shared_ptr<Animator::Animator> findAnimator(std::string object_name) {
        auto it = animators.find(object_name);
        if (it != animators.end()) {
            return it->second;
        }
        else {
            return nullptr;
        }
    }
private:
    std::map<std::string, std::shared_ptr<Animator::Animator>> animators;
};


#endif //ROKIDOPENXRANDROIDDEMO_ANIMATIONPLAYER_H