#include "CameraTracking.h"
#include "debug_utils.hpp"
//using namespace RvgCamTr;

#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <opencv2/core/eigen.hpp>
#include <unistd.h>
#include <thread>


#include <cstring>  // for memcpy

cv::Matx44f convertMatToMatx(const cv::Mat& mat) {
    // 检查 cv::Mat 是否是 4x4 并且类型为 CV_32F
    if (mat.rows != 4 || mat.cols != 4 || mat.type() != CV_32F) {
        throw std::invalid_argument("Input Mat must be 4x4 with type CV_32F.");
    }

    // 创建一个 Matx44f 对象
    cv::Matx44f matx;

    // 使用 memcpy 快速复制数据
    std::memcpy(matx.val, mat.ptr<float>(), 16 * sizeof(float));

    return matx;
}


CameraTracking::CameraTracking() = default;

CameraTracking::~CameraTracking() = default;

cv::Mat convert_to_CV_64F(const cv::Mat& pose_)
{
    cv::Mat pose;
    if (pose_.type() != CV_64F)
    {
        pose_.convertTo(pose, CV_64F);
    }
    else
    {
        pose = pose_;
    }
    return pose;
}

cv::Mat rgbd_pose_normal_pose_exchange(cv::Mat pose){
    assert(pose.rows == 4 && pose.cols == 4);
    assert(pose.type() == CV_32F);
    cv::Mat r = pose(cv::Rect(0, 0, 3, 3)).clone();
    cv::Mat t = pose(cv::Rect(3, 0, 1, 3)).clone();
    cv::Mat r_t = cv::Mat::eye(4, 4, CV_32F);
    r = r.t();
    r.copyTo(r_t(cv::Rect(0, 0, 3, 3)));
    t.copyTo(r_t(cv::Rect(3, 0, 1, 3)));
    return r_t;
}

// (R,t)->(R,-R*t)
cv::Mat R_Rt(cv::Mat pose){
    assert(pose.rows == 4 && pose.cols == 4);
    assert(pose.type() == CV_32F);
    cv::Mat r = pose(cv::Rect(0, 0, 3, 3)).clone();
    cv::Mat t = pose(cv::Rect(3, 0, 1, 3)).clone();
    cv::Mat r_t = cv::Mat::eye(4, 4, CV_32F);
    r.copyTo(r_t(cv::Rect(0, 0, 3, 3)));
    cv::Mat(-r * t).copyTo(r_t(cv::Rect(3, 0, 1, 3)));
    return r_t;
}

cv::Matx44f coord_convert(const cv::Matx44f matrix){
    // @add by wgy&cxx
    // convert CameraPoseMatrix to Render need
    cv::Matx44f matrixT(1, 0, 0, -matrix(0, 3),
                0, 1, 0, -matrix(1, 3),
                0, 0, 1, -matrix(2, 3),
                0, 0, 0, 1);
    cv::Matx44f matrixR(matrix(0, 0), matrix(0, 1), matrix(0, 2), 0,
                        matrix(1, 0), matrix(1, 1), matrix(1, 2), 0,
                        matrix(2, 0), matrix(2, 1), matrix(2, 2), 0,
                        0, 0, 0, 1);
    cv::Matx44f matrixRT = matrixR * matrixT;  // [R|t] -> [R|-Rt], equal to R'=R^-1
    // cp.transform.setPose(matrixRT);
    return matrixRT;
}

cv::Matx44f coord_convert_R(const cv::Matx44f matrix, const cv::Matx44f use_R){
    // @add by wgy&cxx
    // convert CameraPoseMatrix to Render need
    cv::Matx44f matrixT(1, 0, 0, -matrix(0, 3),
                0, 1, 0, -matrix(1, 3),
                0, 0, 1, -matrix(2, 3),
                0, 0, 0, 1);
    cv::Matx44f matrixR(use_R(0, 0), use_R(0, 1), use_R(0, 2), 0,
                        use_R(1, 0), use_R(1, 1), use_R(1, 2), 0,
                        use_R(2, 0), use_R(2, 1), use_R(2, 2), 0,
                        0, 0, 0, 1);
    cv::Matx44f matrixRT = matrixR * matrixT;  // [R|t] -> [R|-Rt], equal to R'=R^-1
    // cp.transform.setPose(matrixRT);
    for(int i=0; i<3; ++i){
        for(int j=0; j<3; ++j){
            matrixRT(i, j) = matrix(i, j);
        }
    }
    return matrixRT;
}


int CameraTracking::Init(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) {
    std::cout << "CameraTrackingModule init start" << std::endl;

    // remote init
    // SerilizedObjs initCmdSend = {
    //     {"cmd",string("init")},
    //     {"isLoadMap", int(appData.isLoadMap)},
    //     {"isSaveMap", int(appData.isSaveMap)}
    // };
    SerilizedObjs initCmdSend = {
        {"cmd",std::string("init")},
        {"isLoadMap", int(appData.isLoadMap)},
        {"isSaveMap", int(appData.isSaveMap)}
    };
    app->postRemoteCall(this, nullptr, initCmdSend);


    sys = std::make_shared<RvgVio::RvgVioSystem>();

    std::string engineDir;
    try{
        engineDir=appData.engineDir;
    }catch(const std::bad_any_cast& e){
        std::cout << e.what() << std::endl;
    }
    std::string trackingConfigPath = engineDir + "/CameraTracking/config.json";

    ConfigLoader crdr(trackingConfigPath);
    std::string configName = crdr.getValue<std::string>("configName");
    std::string configPath = crdr.getValue<std::string>("configPath");
    fps = crdr.getValue<int>("fps");
    std::string syspath = engineDir+configPath;

    std::cout << "CameraTrackingModule syspath:   "<< syspath << std::endl;
    std::cout << "CameraTrackingModule configName:"<< configName << std::endl;

    bool status = sys->Init(syspath, configName);

    indexImu = 0;
    hasImage = false;
    useDepth = true;

    alignTransform = cv::Mat::eye(4, 4, CV_32F);
    alignTransformLast = cv::Mat::eye(4, 4, CV_32F);
    alignTransformLastFile = appData.dataDir + crdr.getValue<std::string>("alignTransformLastFile");
    // load last
    if(appData.isLoadMap){
        if (access(alignTransformLastFile.c_str(), 0) == 0)
        {
            std::ifstream in(alignTransformLastFile);
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    in >> alignTransformLast.at<float>(i, j);
                }
            }
            in.close();
        }
        else
        {
            std:: cerr << "file alignTransformLast.txt deos not exist" << std::endl;
        }
    }

    

    return STATE_OK;
}

int CameraTracking::Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr) {
    static bool firstUpdate = true;
    if(firstUpdate){
        firstUpdate = false;
        std::cout << "waiting for remote init" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        std::cout << "waiting done" << std::endl;
    }

    std::cout << "CameraTrackingModule update start" << std::endl;

    if(sys->isInitialized()){
        std::cout << "imu init finished" << std::endl;
        
    }
    std::vector<std::vector<double>>& imuData = sceneData.imuBuffer;
    time = frameDataPtr->timestamp;
    // indexImu = 0;  //  if sceneData.imuBuffer.clear();
    while(indexImu < (int)imuData.size())
    {
        sys->processIMU(imuData[indexImu]);
        indexImu++;
    }

    imgColor = frameDataPtr->imgColor.clone();
    if(useDepth)
    {
        imgDepth = frameDataPtr->imgDepth.clone();
    }
    hasImage = true;

    // Fill our buffer if we have not (first image)
    if (hasImage && imgColorBuffer.rows == 0)
    {
        hasImage = false;
        timeBuffer = time;
        imgColorBuffer = imgColor.clone();
        if(useDepth)
        {
            imgDepthBuffer = imgDepth.clone();
        }
    }

    // If we are in monocular mode, then we should process the left if we have it
    if (hasImage)
    {

        /// send image data to ours vio system
        if(useDepth) // RGBD mode
        {
            sys->processRGBD(timeBuffer, imgColorBuffer, imgDepthBuffer);
        }
        else  // RGB mode
        {
            // sys->processRGB(timeBuffer, imgColorBuffer);
        }

        // save camera pose

        //CameraPose cp;
        auto& cp = sceneData.getMainCamera();
        cp.timestamp = timeBuffer;
        cv::Matx44f slam_pose_ = sys->getPose().clone();
        // debug
        save_pose_as_tum("/workspace/1_origin_slam_output.txt", time, cv::Mat(slam_pose_));

        cv::Matx44f slam_pose_1 = coord_convert(slam_pose_);
        save_pose_as_tum("/workspace/2_slam_transformed.txt", time, cv::Mat(slam_pose_1));


        cv::Mat slam_pose = cv::Mat(slam_pose_);
        slam_pose = cv::Mat((rgbd_pose_normal_pose_exchange(slam_pose)));
        // debug
        save_pose_as_tum("/workspace/3_slam_transformed_Rnormal.txt", time, cv::Mat(slam_pose));


        cv::Mat camera_tracking_pose;
        if (!appData.isLoadMap)
        {
            camera_tracking_pose = slam_pose;
        }
        else
        {
            std::cout  << "alignTransformLast.inv(): "<< alignTransformLast.inv() << std::endl;
            std::cout  << "alignTransform: "<< alignTransform << std::endl;
            camera_tracking_pose = alignTransformLast.inv() * alignTransform * slam_pose;
        }
        // debug
        save_pose_as_tum("/workspace/update_slam_pose.txt", time, cv::Mat(slam_pose));
        save_pose_as_tum("/workspace/update_alignTransform_x_slam_pose.txt", time, cv::Mat(alignTransform * slam_pose));
        // save_pose_as_tum("/workspace/update_alignTransformLast.inv_x_alignTransform_x_slam_pose.txt", time, cv::Mat(alignTransformLast.inv() * alignTransform * slam_pose));
    
        std::cout << "time: " << time << std::endl;
        std:: cout << "alignTransform: " << alignTransform << std::endl;
        cp.transform.setPose(R_Rt(rgbd_pose_normal_pose_exchange(camera_tracking_pose)));
        save_pose_as_tum("/workspace/final_output.txt", time, R_Rt(rgbd_pose_normal_pose_exchange(camera_tracking_pose)));
        

        // debug
        // std::string savetum;
        // if (appData.isLoadMap)
        // {
        //     savetum = "/workspace/CamTracking_loadmap1_savemap0.txt";
        // }
        // else
        // {
        //     savetum = "/workspace/CamTracking_loadmap0_savemap1.txt";
        // }
        // static bool fisrtFrame = true;
        // if (fisrtFrame)
        // {
        //     fisrtFrame = false;
        //     if (access(savetum.c_str(), 0) == 0)
        //     {
        //         std::cout << "remove " << savetum << std::endl;
        //         remove(savetum.c_str());
        //     }
        // }
        // save_pose_as_tum(savetum, time, cv::Mat(camera_tracking_pose));

        // move buffer forward
        hasImage = false;
        timeBuffer = time;
        imgColorBuffer = imgColor.clone();
        if(useDepth)
        {
            imgDepthBuffer = imgDepth.clone();
        }
    }  //hasImage

    


    // std::cout << "CameraTracking update end" << std::endl;
    return STATE_OK;
}

void CameraTracking::PreCompute(std::string configPath) {
    return;
}

#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <opencv2/core/eigen.hpp>
#include <unistd.h>

//需要检测时，通过RPC调用服务器上的检测功能
int CameraTracking::CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr)
{

    serilizedFrame.addRGBImage(*frameDataPtr);  //添加RGB图像到serilizedFrame。serilizedFrame随后将被上传到服务器。
    serilizedFrame.addDepthImage(*frameDataPtr);  

    // add slam_pose
    SerilizedObjs send = {
        {"cmd", std::string("reloc")},
        {"slam_pose", Bytes(rgbd_pose_normal_pose_exchange(cv::Mat(sys->getPose())))}, //继续添加其它数据
        {"tframe", Bytes(frameDataPtr->timestamp)}
    };
    std::cout << "frameID: " << frameDataPtr->frameID << std::endl;
    // std::cout << "slam_pose: " << sys->getPose() << std::endl;

    // test if timestamp is correct
    static double last_tframe = 0;
    if (frameDataPtr->timestamp < last_tframe)
    {
        std::cerr << "tframe < last_tframe" << std::endl;
        return STATE_ERROR;
    }
    last_tframe = frameDataPtr->timestamp;

    procs.push_back(std::make_shared<RemoteProc>(this, frameDataPtr, send)); //添加到procs，随后该命令将被发送到ObjectTrackingServer进行处理
    return STATE_OK;
}

int CameraTracking::ProRemoteReturn(RemoteProcPtr proc){
    std::cout << "into ProRemoteReturn... " << std::endl;
    auto& send = proc->send;
    auto& ret = proc->ret;
    auto cmd = send.getd<std::string>("cmd");
    
    // printf("ProRemoteReturn: TestPro1 cmd=%s, frameID=%d\n", cmd.c_str(), proc->frameDataPtr->frameID);
    if (cmd == "reloc")
    {//处理reloc命令返回值
        // cv::Mat alignTransform;
        if(ret.getd<bool>("align_OK")){
            alignTransform = ret.getd<cv::Mat>("alignTransform");
        }
        else{
            alignTransform = cv::Mat::eye(4, 4, CV_32F);;
        }
        // std::cout << "alignTransform: " << alignTransform << std::endl;

    }
    return STATE_OK;
}

int CameraTracking::ShutDown(AppData& appData,  SceneData& sceneData){
    if(!has_shutdown){
        has_shutdown = true;
    }
    else{
        return STATE_OK;
    }
    // save current alignTransform as  lastalignTransform if not saved yet
    std::cout << "into CameraTracking ShutDown" << std::endl;
    if(access(alignTransformLastFile.c_str(), 0) == -1){
        std::cout << "save alignTransform in " << alignTransformLastFile << std::endl;
        std::ofstream out(alignTransformLastFile, std::ios::out);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                std::cout << alignTransform.at<float>(i, j) << " ";
                out << alignTransform.at<float>(i, j) << " ";
            }
            out << std::endl;
        }
    }

    // remote shutdown
    SerilizedObjs cmdsend = {
        {"cmd", std::string("shutdown")}
    };
    app->postRemoteCall(this, nullptr, cmdsend);
    
    return STATE_OK;

}