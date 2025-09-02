#include "../include/CameraTracking.h"
#include "../include/debug_utils.hpp"
//using namespace RvgCamTr;

#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
//#include <Eigen/Core>
//#include <Eigen/Dense>
#include "../eigen-3.2.10/Eigen/Core"
#include "../eigen-3.2.10/Eigen/Dense"
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


cv::Mat rgbd_pose_normal_pose_exchange(cv::Mat pose){
    assert(pose.rows == 4 && pose.cols == 4);
    if (pose.type() != CV_32F)
    {
        pose.convertTo(pose, CV_32F);
    }
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

bool checkConstant(std::vector<cv::Mat> vAlignTransform){
    // std:: cout << "into checkConstant..." << std::endl;
    cv::Mat v_tmp = cv::Mat_<float>(vAlignTransform.size(), 1);
    for (int pos_index = 0 ;pos_index < 3; pos_index ++){
        for (int i = 0; i < 30 * 5; i++)
        {
            // if conclude nan, return false
            if (std::isnan(vAlignTransform[i].at<float>(pos_index, 3)))
            {
                return false;
            }

            v_tmp.at<float>(i, 0) = vAlignTransform[i].at<float>(pos_index, 3);
            // debug
            // std::cout << "v_tmp.at<float>(i, 0): " << v_tmp.at<float>(i, 0) << std::endl;
        }
        float var = cv::sum((v_tmp - cv::mean(v_tmp)).mul(v_tmp - cv::mean(v_tmp)))[0] / (30 * 5);
        // debug
        std::cout << "var: " << var << std::endl;
        if (var > 0.01){
            return false;
        }
    }
    return true;
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

    // 根据新增的 sensorType 配置决定是否使用深度
    {
        uint sensorType = crdr.getValue<int>("sensorType");  // 0=MONOCULAR, 2=RGBD
        SerilizedObjs initCmdSend = {
            {"cmd", std::string("init")},
            {"isLoadMap", int(appData.isLoadMap)},
            {"isSaveMap", int(appData.isSaveMap)},
            {"sensorType", int(sensorType)}
        };
        app->postRemoteCall(this, nullptr, initCmdSend);
    }

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
        // move buffer forward. TODO(lsw)：这里似乎没有必要
        hasImage = false;
        timeBuffer = time;
        imgColorBuffer = imgColor.clone();
        if(useDepth)
        {
            imgDepthBuffer = imgDepth.clone();
        }

        /// send image data to ours vio system
        if(useDepth) // RGBD mode
        {
            sys->processRGBD(timeBuffer, imgColorBuffer, imgDepthBuffer);
        }
        else  // RGB mode
        {
            // sys->processRGB(timeBuffer, imgColorBuffer);
        }

        //CameraPose cp;
        auto& cp = sceneData.getMainCamera();
        cp.timestamp = timeBuffer;
        cv::Matx44f slam_pose_ = sys->getPose().clone();
        // debug
        if(debugging) save_pose_as_tum(debug_output_path + "/1_origin_slam_output.txt", time, cv::Mat(slam_pose_));

        cv::Matx44f slam_pose_1 = coord_convert(slam_pose_);
        if(debugging) save_pose_as_tum(debug_output_path + "/2_slam_transformed.txt", time, cv::Mat(slam_pose_1));


        cv::Mat slam_pose = cv::Mat(slam_pose_);
        slam_pose = cv::Mat((rgbd_pose_normal_pose_exchange(slam_pose)));
        // debug
        if(debugging) save_pose_as_tum(debug_output_path + "/3_slam_transformed_Rnormal.txt", time, cv::Mat(slam_pose));


        cv::Mat camera_tracking_pose; //T_wc 具有坐标变换意义的位姿
        if (!appData.isLoadMap)
        {
            camera_tracking_pose = slam_pose;
        }
        else
        {
            while(true){            
                if (frameID2RelocPose.find(frameDataPtr->frameID) == frameID2RelocPose.end()){ 
                    usleep(1000*33); // wait for one frame
                }
                else{
                    // 一直等到到有值，所以不会为空
                    if (!appData.bUseRelocPose)
                        camera_tracking_pose = alignTransformLast.inv() * alignTransform * slam_pose;
                    else{
                        camera_tracking_pose = alignTransformLast.inv() * frameID2RelocPose[frameDataPtr->frameID];
                    }
                    break;
                }
            }
        }
        
        cp.transform.setPose(R_Rt(rgbd_pose_normal_pose_exchange(camera_tracking_pose)));
        save_pose_as_tum(debug_output_path + "/output.txt", time, camera_tracking_pose); // 给重建模块的位姿


    }  //hasImage

    


    return STATE_OK;
}

void CameraTracking::PreCompute(std::string configPath) {
    return;
}

#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
//#include <Eigen/Core>
//#include <Eigen/Dense>
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
    auto& send = proc->send;
    auto& ret = proc->ret;
    auto cmd = send.getd<std::string>("cmd");
    
    if (cmd == "reloc")
    {//处理reloc命令返回值
        if(ret.getd<bool>("align_OK")){
            alignTransform = ret.getd<cv::Mat>("alignTransform");

            // 重定位需要一直开启
            // // check if need to close remote
            // // std::cout << "vAlignTransform.size(): " << vAlignTransform.size() << std::endl;
            // if(vAlignTransform.size() < 30 * 5){
            //     vAlignTransform.push_back(alignTransform.clone());
            // }
            // else{
            //     vAlignTransform.erase(vAlignTransform.begin());
            //     vAlignTransform.push_back(alignTransform);
            //     std::cout << "checkConstant(vAlignTransform): " << checkConstant(vAlignTransform) << std::endl;
            //     if (checkConstant(vAlignTransform) && app->appData->isLoadMap){
            //         // yellow cout
            //         std::cout << "\033[33m" << "alignTransform is constant, shutdown remote" << "\033[0m" << std::endl;
            //         // remote shutdown
            //         SerilizedObjs cmdsend = {
            //             {"cmd", std::string("shutdown")}
            //         };
            //         app->postRemoteCall(this, nullptr, cmdsend);
            // }
            // }

        }
        else{
            alignTransform = cv::Mat::eye(4, 4, CV_32F);;
        }
        // std::cout << "alignTransform: " << alignTransform << std::endl;
        frameID2RelocPose[ret.getd<int>("curFrameID")] = ret.getd<cv::Mat>("RelocPose");
        std::cout << "frameID : " << ret.getd<int>("curFrameID") << std::endl;

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
    std::cout << "client CameraTracking send shutdown to Server" << std::endl;
    
    return STATE_OK;
}