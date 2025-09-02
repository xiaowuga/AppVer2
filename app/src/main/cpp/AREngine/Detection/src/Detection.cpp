
#include"Detection/include/Detection.h"
#include"Detection/include/SegAndPoseEstimation.h"
#include"Basic/include/ConfigLoader.h"

InputRealSense::InputRealSense()
	:_impl(new RSLoader)
{}

int InputRealSense::Init(AppData& appData, FrameData& frameData, SceneData& sceneData)
{
	if (!_impl)
	{
		auto cfgFile = appData.engineDir + "/InputRealSense/config.json";
		ConfigLoader cfg(cfgFile);

		int width = cfg.getValue<int>("width");
		int height = cfg.getValue<int>("height");
		float fps = cfg.getValue<float>("fps");
		_impl = std::make_shared<RSLoader>(width, height, fps);
	}
	return STATE_OK;
}

int InputRealSense::Update(AppData& appData, FrameData& frameData, SceneData& sceneData)
{
	return _impl->updateFrame(frameData, sceneData) ? STATE_OK : S_ERROR;
}

