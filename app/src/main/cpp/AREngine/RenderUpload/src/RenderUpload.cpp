#include "RenderUpload.h"
// #include "debug_utils.hpp"

#include <string>
#include <fstream>
#include <unistd.h>
#include <thread>
#include <chrono>

#include <cstring>

#include <android/log.h>
#define LOG_TAG "RenderUpload.cpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

using namespace cv;
using namespace std;

static const double UPLOAD_INTERVAL = 1.0 / 30.0; // ~30fps


int RenderUpload::Init(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) {
    LOGI("RenderUpload init");
    SerilizedObjs cmdSend = {
            {"cmd", std::string("initARCloudRenderer")},
            {"width", 640},
            {"height", 480},
            {"upsample_scale", 2},
            {"KParameters", std::vector<double>{281.60213015, 281.37377039, 318.69481832, 243.6907021}},
            {"model_transforms", sceneData.model_transforms_vector},
            {"instance_names", sceneData.instance_names},
            {"model_paths", sceneData.model_paths}
    };
    app->postRemoteCall(this, nullptr, cmdSend);
    LOGI("RenderUpload init done");
    sceneData.render_init_done = true;
    return STATE_OK;
}

int RenderUpload::Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) {
    // 30fps throttle
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - _lastUploadTime).count();
    if (elapsed < UPLOAD_INTERVAL) {
        return STATE_OK;
    }
    _lastUploadTime = now;

    // build upload data: base models + animation data (if any)
    std::vector<std::vector<double>> transforms;
    std::vector<std::string> names;

    {
        std::lock_guard<std::mutex> lock(sceneData.renderUploadLock);
        // copy base models
        int baseCount = sceneData.baseModelCount;
        transforms.assign(sceneData.model_transforms_vector.begin(),
                          sceneData.model_transforms_vector.begin() + std::min(baseCount, (int)sceneData.model_transforms_vector.size()));
        names.assign(sceneData.instance_names.begin(),
                     sceneData.instance_names.begin() + std::min(baseCount, (int)sceneData.instance_names.size()));

        // append animation data if available
        if (sceneData.hasAnimationData && !sceneData.animation_transforms_buffer.empty()) {
            transforms.insert(transforms.end(),
                              sceneData.animation_transforms_buffer.begin(),
                              sceneData.animation_transforms_buffer.end());
            names.insert(names.end(),
                         sceneData.animation_names_buffer.begin(),
                         sceneData.animation_names_buffer.end());
        }
    }

    SerilizedObjs cmdSend = {
            {"cmd",              std::string("drawARCommand")},
            {"project",          sceneData.project},
            {"view",             sceneData.view},
            {"model_transforms", transforms},
            {"instance_names",   names}
    };

    app->postRemoteCall(this, frameDataPtr,
                        cmdSend); //发送set命令
    return STATE_OK;
}


int RenderUpload::CollectRemoteProcs(SerilizedFrame &serilizedFrame,
                                  std::vector<RemoteProcPtr> &procs, FrameDataPtr frameDataPtr) {
//    SerilizedObjs cmdSend = {
//            {"cmd", std::string("RenderUploadGlass")}
//    };
//
//    procs.push_back(std::make_shared<RemoteProc>(this,frameDataPtr,cmdSend,
//                                                 RPCF_SKIP_BUFFERED_FRAMES));
    return STATE_OK;
}
int RenderUpload::ProRemoteReturn(RemoteProcPtr proc) {
    auto& send = proc->send;
    auto& ret = proc->ret;
    auto cmd = send.getd<std::string>("cmd");
    if (cmd == "initARCloudRenderer") {
//        LOGI()
    }
    return STATE_OK;
}

int RenderUpload::ShutDown(AppData &appData, SceneData &sceneData)
{
    return STATE_OK;
}
