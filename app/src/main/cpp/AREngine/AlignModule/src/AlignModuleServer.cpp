
#include"AlignModule/include/AlignModuleServer.h"
using namespace cv;

int AlignModuleServer::init(RPCServerConnection& con)
{
	//��ʼ��
	//APP����ʱ��Ѵ���ϴ�������������˴����ļ���config�ļ�����������ȣ����Բ��úͿͻ���ͬ���ķ�ʽ����
	auto& appData = _app->appData;
	auto& sceneData = _app->sceneData;

	auto cfgFile = appData->engineDir + "/AlignModule/config.json";

	//....���ؼ��ģ�͵�

	return STATE_OK;
}

//ִ�й��̵���
int AlignModuleServer::call(RemoteProcPtr proc, FrameDataPtr frameDataPtr, RPCServerConnection& con)
{
	auto& send = proc->send;
	
	auto cmd = send.getd<std::string>("cmd");

	if (cmd == "detectPlane")
	{
		// save RGB & D & pose.txt

		// ...


		// system.cmd
		std::string cmd = "mkdir -p " + this->base_path + "/cam0/data";
		int ret;
		ret = system(cmd.c_str());
		// ...


		// read reuslt plane txt file & return 
		// ...
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

