
#include"CameraTracking/include/CameraTrackingServer.h"
#include "debug_utils.hpp"
#include <mutex>
#include <unistd.h>
#include <sys/stat.h>// file size
using namespace cv;

std::mutex mtx;

// CameraTrackingServer::CameraTrackingServer()
// {
	
// }

int CameraTrackingServer::init(RPCServerConnection& con)
{
	return STATE_OK;
}

//ִ�й��̵���
int CameraTrackingServer::call(RemoteProcPtr proc, FrameDataPtr frameDataPtr, RPCServerConnection& con)
//������Ӧ���ӵ���Ϣ
{
	auto& send = proc->send;
	auto cmd = send.getd<std::string>("cmd");
	std::cout << "cmd: " << cmd << std::endl;
	// std::cout << "frameID: " << frameDataPtr->frameID << std::endl;

	// debug
	static std::vector<double> vSlamTimestamps;
	static std::vector<cv::Mat> vSlamPoses;

	if(cmd == "init"){
		auto& appData = con.app->appData;
		appData->isLoadMap = send.getd<bool>("isLoadMap", 0);
		appData->isSaveMap = send.getd<bool>("isSaveMap", 0);
		std::cout << "appData:isLoadMap" << appData->isLoadMap << std::endl;
		std::cout << "appData:isSaveMap" << appData->isSaveMap << std::endl;
			//��ʼ��
		//APP����ʱ��Ѵ���ϴ�������������˴����ļ���config�ļ�����������ȣ����Բ��úͿͻ���ͬ���ķ�ʽ����
		// auto& appData = _app->appData;
		auto& sceneData = _app->sceneData;

		// auto cfgFile = appData->engineDir + "/CameraTracking/config.json";

		//....���ؼ��ģ�͵�

		std::cout << "init start" << std::endl;
		sys = std::make_shared<Reloc::Relocalization>();

		
		std::string engineDir;
		try{
			engineDir=appData->engineDir;
		}catch(const std::bad_any_cast& e){
			std::cout << e.what() << std::endl;
		}
		std::string trackingConfigPath = engineDir + "CameraTracking/config.json";

		ConfigLoader crdr(trackingConfigPath);
		std::string configPath = crdr.getValue<std::string>("configPath");
		std::string configName = appData->engineDir + configPath + crdr.getValue<std::string>("reloc_configName");
		std::string vocaFile = appData->engineDir + configPath + crdr.getValue<std::string>("vocaFile");
		std::string mapFile = appData->dataDir + crdr.getValue<std::string>("mapFile");

		if (access(appData->dataDir.c_str(), 0) == -1)	
		{	
			system(("mkdir -p " + appData->dataDir).c_str());
			std::cout << "dataDir does not exist, create : " << appData->dataDir << std::endl;
		}
		// check if mapfile is empty
		if (access(mapFile.c_str(), 0) == 0)
		{
			struct stat statbuf;
			stat(mapFile.c_str(), &statbuf);
			std::cout << "mapFile size: " << statbuf.st_size << std::endl;
			if (statbuf.st_size == 0)
			{	
				// red color cerr
				std::cerr << "\033[31m" << "mapFile is empty, please check the mapFile path or delete it: " << mapFile << "\033[0m" << std::endl;
				return STATE_ERROR;
			}
		}


		bool status = sys->Init(configName, vocaFile, mapFile, appData->isLoadMap, appData->isSaveMap);
		try {
			if (!status) {
				throw std::runtime_error("Reloc Initialization failed.");
			}
		}catch (const std::runtime_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return STATE_ERROR;
		
		}
		initialized = true;
		std::cout << "CameraTrackingServer init successfully" << std::endl;
	}
	else if (cmd == "reloc")
	{
		mtx.lock();

		auto rgb = frameDataPtr->image.front().clone();
		auto depth = frameDataPtr->depth.front().clone();
		auto slam_pose = send.getd<cv::Mat>("slam_pose");
		auto tframe = send.getd<double>("tframe");

		CV_Assert(rgb.size()==Size(640,480)); //��֤����ͼ���С������ֵ

		cv::Mat slam_pose_32f;
		slam_pose.convertTo(slam_pose_32f, CV_32F);
		sys->processRGBD_Pose(tframe, rgb, depth, slam_pose_32f);
		cv::Mat alignTransform;
		bool align_OK = sys->alignSLAM2Reloc(alignTransform);
		std::cout <<"time: " << tframe << std::endl;
		std::cout << "alignTransform: " << alignTransform << std::endl;
		proc->ret = {
			{"align_OK",Bytes(align_OK)},
			{"alignTransform",Bytes(alignTransform)}
		};

		// debug
		vSlamTimestamps.push_back(tframe);
		vSlamPoses.push_back(slam_pose.clone());
		// check frameID
		static int last_frameID = -1;
		if (frameDataPtr->frameID < last_frameID)
		{
			std::cout << "\033[31m" << "frameDataPtr->frameID < last_frameID" << "\033[0m" << std::endl;
			std::cout << "frameDataPtr->frameID: " << frameDataPtr->frameID << std::endl;
			std::cout << "last_frameID: " << last_frameID << std::endl;
		}
		last_frameID = frameDataPtr->frameID;
		// check tframe
		static double last_tframe = 0;
		if (tframe < last_tframe)
		{
			std::cerr << "tframe < last_tframe" << std::endl;
			return STATE_ERROR;
		}
		last_tframe = tframe;

		mtx.unlock();
	}
	else if (cmd == "shutdown")
	{
		std::cout << "into shutdown..." << std::endl;
		// mtx.lock();

		// debug
		std::string SLAMPoseFile = "/workspace/ServerSLAMPose.txt";
		std::string RelocPoseFile = "/workspace/ServerRelocPose.txt";
		// remove pose files
		static bool first_time = true;
		if (first_time)
		{
			first_time = false;
			if (access(SLAMPoseFile.c_str(), 0) == 0)
			{
				system(("rm " + SLAMPoseFile).c_str());
				std::cout << "remove " << SLAMPoseFile << std::endl;
			}
			if (access(RelocPoseFile.c_str(), 0) == 0)
			{
				system(("rm " + RelocPoseFile).c_str());
				std::cout << "remove " << RelocPoseFile << std::endl;
			}
		}
		// save SLAM poses
		if (vSlamTimestamps.size() != vSlamPoses.size())
		{
			std::cerr << "vSlamTimestamps.size() != vSlamPoses.size()" << std::endl;
			return STATE_ERROR;
		}
		for (int i = 0; i < vSlamTimestamps.size(); i++)
		{
			double timestamp = vSlamTimestamps[i];
			cv::Mat pose = vSlamPoses[i];
			// std::cout << "timestamp: " << timestamp << std::endl;
			// std::cout << "pose: " << pose << std::endl;
			save_pose_as_tum(SLAMPoseFile, timestamp, pose);
		}
		// save Reloc poses
		std::cout << "getRelocPoses..." << std::endl;
		std::vector<cv::Mat> vRelocPoses = sys->getPoses();
		std::cout << "vRelocPoses.size(): " << vRelocPoses.size() << std::endl;
		for (int i = 0; i < vRelocPoses.size(); i++)
		{
			double timestamp = vSlamTimestamps[i];
			cv::Mat pose = vRelocPoses[i];
			// std::cout << "pose: " << pose << std::endl;
			save_pose_as_tum(RelocPoseFile, timestamp, pose);
		}

		// shutdown
		sys->Shutdown();
		std::cout << "CameraTrackingServer shutdown successfully" << std::endl;
		// mtx.unlock();
	}
	return STATE_OK;
}

