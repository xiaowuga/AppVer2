
#include"InputRealSense/include/InputRealSense.h"
#include"InputRealSense/include/realsense.h"
#include"Basic/include/ConfigLoader.h"

InputRealSense::InputRealSense() : _impl(nullptr) {}

int InputRealSense::Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) {
    if (!_impl) {
        auto cfgFile = appData.engineDir + "InputRealSense/config.json";
        ConfigLoader cfg(cfgFile);

        int width = cfg.getValue<int>("width");
        int height = cfg.getValue<int>("height");
        float fps = cfg.getValue<float>("fps");
        _impl = std::make_shared<RSLoader>(width, height, fps);
        std::cout << "value of record"<< appData.record<< std::endl;

        // if(appData.record){
        //     _impl->base_path=appData.offlineDataDir;

        //     _impl->initialize_files();
        // }

    }
    std::cout << "InputRealSense Init successed" << std::endl;
    return STATE_OK;
}

int InputRealSense::Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) {
    // std::cout << "InputRealSense Update successed" << std::endl;
    return _impl->updateFrame(appData, frameDataPtr, sceneData) ? STATE_OK : STATE_ERROR;
}

int InputRealSense::ShutDown(AppData& appData, SceneData& sceneData){
    std::cout << "InputRealSense ShutDown successed" << std::endl;
    // if(appData.record){
    //     _impl->save_all_data();
    //     std::cout << "InputRealSense ShutDown----save ok" << std::endl;

    // }

    return _impl->ShutDown() ? STATE_OK : STATE_ERROR;
}
