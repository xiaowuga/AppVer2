
#include"ObjectTracking/include/ObjectTrackingServer.h"
using namespace cv;

int ObjectTrackingServer::init(RPCServerConnection& con)
{
	//��ʼ��
	//APP����ʱ��Ѵ���ϴ�������������˴����ļ���config�ļ�����������ȣ����Բ��úͿͻ���ͬ���ķ�ʽ����
	auto& appData = _app->appData;
	auto& sceneData = _app->sceneData;

	auto cfgFile = appData->engineDir + "/ObjectTracking/config.json";

	//....���ؼ��ģ�͵�

	return STATE_OK;
}

//ִ�й��̵���
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
		//...ִ�м��

		//���÷��ؽ������������ARModule::ProRemoteReturn�н��ղ����д���
		proc->ret = {
			{"results",results}
		};
	}

	//��������
	if (cmd == "other")
	{
	}

	return STATE_OK;
}

