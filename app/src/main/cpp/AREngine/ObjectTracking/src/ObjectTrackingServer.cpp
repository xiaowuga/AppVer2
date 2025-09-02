
#include"ObjectTracking/include/ObjectTrackingServer.h"
using namespace cv;

int ObjectTrackingServer::init(RPCServerConnection& con)
{
	//初始化
	//APP发布时会把打包上传到服务器，因此磁盘文件（config文件，场景对象等）可以采用和客户端同样的方式访问
	auto& appData = _app->appData;
	auto& sceneData = _app->sceneData;

	auto cfgFile = appData->engineDir + "/ObjectTracking/config.json";

	//....加载检测模型等

	return STATE_OK;
}

//执行过程调用
int ObjectTrackingServer::call(RemoteProcPtr proc, FrameDataPtr frameDataPtr, RPCServerConnection& con)
{
	auto& send = proc->send;
	
	auto cmd = send.getd<std::string>("cmd");

	if (cmd == "detect")
	{
		Matx34f initPose = send.getd<Matx34f>("initPose", Matx34f::eye());

		if (frameDataPtr->image.empty())
			throw std::runtime_error("empty frame image");
		
		Mat img = frameDataPtr->image[0];
		
		std::vector<DetectedObj> results;
		//...执行检测

		//设置返回结果，随后可以在ARModule::ProRemoteReturn中接收并进行处理
		proc->ret = {
			{"results",results}
		};
	}

	//其它命令
	if (cmd == "other")
	{
	}

	return STATE_OK;
}

