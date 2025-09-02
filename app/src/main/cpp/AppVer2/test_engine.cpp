
#include"Basic/include/RPC.h"
#include"Basic/include/ARModule.h"
#include"Basic/include/App.h"
#include"opencv2/core.hpp"
#include "Basic/include/listener.h"
using namespace cv;
using namespace std;

//模拟相机输入模块
class TestInput
        :public ARModule
{
public:
    int Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr)
    {
        printf("update frame %d\n", frameDataPtr->frameID); //frameID在App::run中维护

        Mat1b img = Mat1b(Size(640, 480));
        img = uchar(frameDataPtr->frameID % 256); //模拟输入，将像素值设置为frameID%256
        frameDataPtr->image.push_back(img);

        //return frameDataPtr->frameID < 50 ? STATE_OK : STATE_ERROR; //50帧后结束
        return STATE_OK;
    }

    int ShutDown(AppData& appData,  SceneData& sceneData){
        return STATE_OK;
    }
};

//模拟AR算法模块
class TestPro1
        :public ARModule
{
    std::vector<int> _retFrames;
    std::mutex   _retFramesMutex;
public:
    int Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (frameDataPtr->hasUploaded("RGB0")) //如果当前帧的RGB图像已经上传（通过CollectRemoteProcs)
            //如果没有上传又必须要进行远程调用，则需要通过send数据上传
            //这样不同模块可能重复上传同样的数据，因此应该尽量在CollectRemoteProcs中发送命令
        {
            SerilizedObjs cmdSend = {
                    {"cmd",string("set")},
                    {"val",int(rand() % 256)}
            };
            app->postRemoteCall(this, frameDataPtr, cmdSend); //发送set命令，将图像的像素值设置为指定的值，并返回结果图像(参考TestServer.cpp中TestPro1Server类的实现)
        }

        while(false)
        {
            {
                std::lock_guard<std::mutex> _lock(_retFramesMutex);
                if (std::find(_retFrames.begin(), _retFrames.end(), frameDataPtr->frameID) != _retFrames.end())
                    break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        return STATE_OK;
    }

    int ShutDown(AppData& appData,  SceneData& sceneData){
        return STATE_OK;
    }

    int CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr)
    {
        auto& frameData = *frameDataPtr;

        serilizedFrame.addRGBImage(frameData, 0, ".png"); //测试代码功能需要无损压缩，用png格式

        SerilizedObjs cmdSend = {
                {"cmd",string("add")},
                {"val",int(rand())}
        };

        /*注意在CollectRemoteProcs中不要用App::postRemoteCall发送命令。
          因为视频帧数据是在所有模块的CollectRemoteProcs执行结束后才上传，因此在这里用App::postRemoteCall将导致命令比视频帧先送达服务器，这样命令将找不到对应的视频帧。
        */
        procs.push_back(std::make_shared<RemoteProc>(this, frameDataPtr, cmdSend, RPCF_SKIP_BUFFERED_FRAMES)); //添加add命令，将输入帧的像素值加上指定的值，并返回结果图像(参考TestServer.cpp中TestPro1Server类的实现)

        return STATE_OK;
    }

    int ProRemoteReturn(RemoteProcPtr proc)
    {
        auto& send = proc->send;
        auto& ret = proc->ret;
        auto cmd = send.getd<std::string>("cmd");

        printf("ProRemoteReturn: TestPro1 cmd=%s, frameID=%d\n", cmd.c_str(), proc->frameDataPtr->frameID);

        if (cmd == "set")
        {//处理set命令返回值
            Mat1b retImg = ret["result"].get<Image>();
            CV_Assert(retImg(0, 0) == (uchar)send["val"].get<int>()); //验证像素值与val参数指定的值相等
        }

        if (cmd == "add")
        {//处理add命令返回值
            Mat1b img = proc->frameDataPtr->image.front();
            Mat1b retImg = ret["result"].get<Image>();

            CV_Assert(img(0, 0) == uchar(proc->frameDataPtr->frameID % 256));

            CV_Assert(retImg(0, 0) == uchar(int(img(0, 0) + send["val"].get<int>()) % 256)); //验证像素值为原像素值与val相加


            int fid = ret.getd<int>("curFrameID");
            std::lock_guard<std::mutex> _lock(_retFramesMutex);
            _retFrames.push_back(proc->frameDataPtr->frameID);
        }

        return STATE_OK;
    }
};

void client_app()
{
    std::string appName = "TestApp"; //APP名称，必须和服务器注册的App名称对应（由服务器上appDir中文件夹的名称确定）

    std::vector<ARModulePtr> modules;
    modules.push_back(createModule<TestInput>("TestInput"));
    modules.push_back(createModule<TestPro1>("TestPro1"));  //用createModule创建模块，必须指定一个模块名，并且和server上的模块名对应！！

    auto appData=std::make_shared<AppData>();
    auto sceneData=std::make_shared<SceneData>();

    appData->argc = 1;
    appData->argv = nullptr;
    appData->engineDir = "./AREngine/";  // for test
    appData->dataDir = "./data/";        // for test

    //std::thread listenThread(listenForEvent, std::ref(*appData));

    ARApp app;
    app.init(appName,appData, sceneData, modules);

    app.connectServer("localhost", 123);
    // app.connectServer("10.102.32.173", 123);
    app.run();
}

void test_rpc_client()
{
    const int N = 1; //模拟N个客户端向服务器并行发送请求

    std::vector<std::shared_ptr<std::thread>> v;
    for (int i = 0; i < N; ++i)
    {
        v.push_back(std::make_shared<std::thread>(client_app));
    }

    for (auto& t : v)
        t->join();
}


void start_test_engine()
{
    std::thread(test_rpc_client).detach();
}

