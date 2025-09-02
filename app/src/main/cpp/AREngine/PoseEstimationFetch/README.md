# PoseEstimation 说明文档

## 环境准备

### 1. onnxruntime-gpu 配置

onnxruntime-gpu 是基于 onnx 模型架构的推理/训练框架，实现了跨平台的高速推理。在 PoseEstimation 中我们使用了 onnxruntime-linux-x64-gpu-1.15.1。依赖如下

- NVIDIA driver 535
- CUDA 11.8，这一依赖与渲染用的 CUDA 相同
- cudnn 8.7.0

需要预先安装好。可以通过编译项目 [cudnn_samples_v8](https://github.com/johnpzh/cudnn_samples_v8) 验证 cudnn 的安装是否成功。

下载 [onnxruntime-linux-x64-gpu-1.15.1.tgz](https://github.com/microsoft/onnxruntime/releases/download/v1.15.1/onnxruntime-linux-x64-gpu-1.15.1.tgz) 之后将其解压在 `AREngine/PoseEstimation/asset` 下。

### 2. 模型下载

模型下载地址为：[百度网盘（提取码: ynht）](https://pan.baidu.com/s/108q7QSFwrQlZnO4l7LHfXA?pwd=ynht)，将其中的模型解压到 `AREngine/PoseEstimation/asset` 下，目录结构为：

```
AREngine/PoseEstimation/asset/
├── gold_yolo_s_hand_0.4246_1x3x480x640.onnx
├── handedlr_mobilenetv3_l.onnx
├── hand_mano.onnx
└── onnxruntime-linux-x64-gpu-1.15.1/
```

## 代码结构

### 1. `InputMVS`

这块的代码是用于调用工业相机进行同步图像捕捉的，代码在 `AREngine/InputMVS` 下面，其核心功能是在每一次调用 `Update` 方法的时候调用相机 API 并将数据写入到 `frameDataPtr->image` (`std::vector<cv::Mat>`) 中。几个相机就会写入几个图片。

### 2. `PoseEstimation.h` / `PoseEstimation.cpp`

> 这是一个样例性质的程序，展示了“数据捕捉-调用姿态服务-结果取回展示”的流程。

`PoseEstimation.{h,cpp}` 这两个文件描述了姿态估计的客户端代码，由于姿态估计的逻辑是跑在服务端的，所以客户端代码只负责：

1. 从 `InputMVS` 收集图像数据对象中，结合时间辍通过引擎的 RPC 机制发送到服务端。这一部分是通过 `CollectRemoteProcs` 处理；
2. 使用 `ProRemoteReturn` 从 RPC 机制中获得姿态估计结果并将其写入到 `FrameData` 和客户端成员 `handPoses` 中；
3. `Update` 方法从 `handPosees` 中读取手部姿态数据并可视化地绘制出来。

需要注意的是，在其中有两行这样的代码
```cpp
proc->frameDataPtr->handPoses.push_bach(HandPose(...));
```
这两行代码的作用是将姿态估计的结果数据推送到整个端云引擎都可以访问的空间中，用于全局的数据共享。

### 3. `PoseEstimationServer.h` / `PoseEstimationServer.cpp`

这是负责姿态估计服务的主体程序。主要关心其 `call` 函数，这一个函数负责了

1. 模型初始化；
2. 姿态估计远程调用
3. 结果打包返回
4. 系统关闭

## 系统运行

在项目根目录下，首先运行

```bash
bash test_T05_PoseEstimationServer.sh
```

等待其运行完成之后会自动拉起服务端，此时打开另一个终端窗口模拟客户端，在其中运行

```bash
bash test_T05_PoseEstimationClient.sh
```

# PoseEstimationFetch 说明文档

PoseEstimationFetch 模块是一个示例模块，描述了如何从 PoseEstimationServer 中使用远程调用的方式获取姿态数据，这部分的调用方式实际上和 `PoseEstimation.{h,cpp}` 中一样。

## 代码结构

该模块的代码主要由如下的部分构成

```
AREngine/PoseEstimation/include/PoseEstimationFetch.h
AREngine/PoseEstimation/src/PoseEstimationFetch.cpp
AREngine/example/T05_PoseEstimationFetch.cpp
```

其中 `PoseEstimationFetch.{h,cpp}` 描述了如何进行远程调用以及返回的姿态信息怎么解码。`T05_PoseEstimationFetch.cpp` 描述了应该把 `"PoseEstimation"` 作为模块如何注册以支持远程调用。

### `T05_PoseEstimationFetch.cpp`

该文件描述了如何将姿态估计服务器拿过来进行姿态信息的获取

```cpp
int main(int argc, char** argv) {
    std::string appName = "PoseEstimationFetch";
    std::vector<ARModulePtr> modules;
    modules.push_back(createModule<PoseEstimationFetch>("PoseEstimation"));

    auto appData=std::make_shared<AppData>();
	auto sceneData=std::make_shared<SceneData>();

    appData->argc = argc;
    appData->argv = argv;
    appData->rootDir = std::string(PROJECT_PATH);
    appData->engineDir = std::string(PROJECT_PATH) + "/AREngine/";  // for test
    appData->dataDir = std::string(PROJECT_PATH) + "/data/";        // for test
    appData->offlineDataDir = std::string(PROJECT_PATH) + "/data/10312218";

	ARApp app;
	app.init(appName, appData, sceneData, modules);
	app.connectServer("localhost", 1203);
	app.run();

    return 0;
}
```

其中最重要的是 `modules.push_back(...)` 部分，`createModule` 部分的模板类型为 `PoseEstimationFetch`，这里注册了一个示例类型，这个示例类型仅用于演示获取姿态信息，而传入参数 `"PoseEstimation"` 与创建姿态估计服务时使用的关键词 `"PoseEstimation"` 一致。

### `PoseEstimationFetch.{h,cpp}`

比较重要的是其中的 `cpp` 文件中的 `Update` 和 `ProRemoteReturn` 方法的定义。

```cpp
int PoseEstimationFetch::Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "fetch pose" << std::endl;
    // 使用FetchPose从远程获取姿态信息
    SerilizedObjs cmdSend = {
        {"cmd", std::string("FetchPose")}
    };
    app->postRemoteCall(this, nullptr, cmdSend);
    return STATE_OK;
}

int PoseEstimationFetch::ProRemoteReturn(RemoteProcPtr proc) {
    // 从远程获得姿态数据
    std::cout << "Get pose from remote" << std::endl;
    auto& send = proc->send;
    auto& ret = proc->ret;
    auto cmd = send.getd<std::string>("cmd");

    if (cmd == "FetchPose") {
        // 标识手部姿态是否有效
        bool handFlag = ret.getd<bool>("handPose_OK");
        // 长度126=2*21*3，表示双手21个关节点的三维姿态
        // 其中0~62是属于左手的关节点信息，63~125是属于右手的关节点信息
        auto handPositionResult = ret.getd<std::vector<float>>("handPosition");
        // 关节点的MANO旋转信息，目前没有用到
        auto rotationResult = ret.getd<std::vector<float>>("handPose");
        // 关节点的的MANO形态信息，目前没有用到
        auto shapeResult = ret.getd<std::vector<float>>("handShape");
        // 关节点在二维图像上的投影位置
        auto joints2dResult = ret.getd<std::vector<float>>("joints_img");
        std::cout << "handFlag: " << handFlag << std::endl;
    }

    return STATE_OK;
}
```