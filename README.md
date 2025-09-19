# Application Verification 2
Application Verification 2 of AR/MR Engine

## Tested Platforms
- Operating System: Windows 11
- Processor: Intel(R) Core i9-13900K (24 cores, 32 threads)
- RAM: 64GB DDR4
- GPU: NVIDIA GeForce RTX 4090
- IDE: Android Studio 2024.3.2 Patch 1
- Android SDK: Version 36.1.0
- Android NDK: Version 29.0.14033849 rc4

## AppVer2Data

将 `appData.appDir` 赋值为文件夹 "AppVer2Data" 的路径后，请将眼镜连接到电脑，并将文件夹 "AppVer2Data" 放置到眼镜主存储的顶层路径中，与文件夹 "Download" 同级。具体路径为 `/storage/emulated/0/`。
您可以在眼镜中的应用商店下载并使用 `CX 文件管理器` 来查看和管理文件。

```
AppVer2Data (appData.appDir)
|-- CameraTracking        # 存放与相机追踪相关的文件
|-- font                  # 存放字体文件
|-- hand                  # 存放手部相关的文件
    |-- handNode_0.fb
    |-- handNode_1.fb
    |-- ...
|-- handNode              # 存放手部节点的文件
|-- Image                 # 存放图像文件
|-- Manual                # 存放手册文件
|-- Models                # 存放模型文件
    |-- folders...          # 模型文件夹
|-- shaders               # 存放着色器文件
|-- textures              # 存放纹理文件
|-- Animation.json        # 动画相关的JSON配置文件
|-- config.ini            # 配置文件，存放基本设置
|-- InstanceInfo.json     # 存放实例信息的JSON文件
|-- InstanceState.json    # 存放实例状态的JSON文件
|-- InteractionConfig.json# 存放交互配置的JSON文件
|-- log.txt               # 日志文件，记录系统运行日志
|-- templ_1.json          # 重定位模板文件1(目前使用)
|-- marker_1.docx         # 重定位模板文件1对应的marker打印文件
|-- templ_2.json          # 重定位模板文件2(备用)
|-- marker_2.docx         # 重定位模板文件2对应的marker打印文件
```

## 谷歌网盘下载连接:
AppVer2Data下载：[AppVer2Data.zip](https://drive.google.com/uc?export=download&id=1vk_Bwio-3JN8-eiqQd9wS8amkkf2D3hy)

如果文件有问题，或者下载存在限制，请与我（江敬恩）联系。
邮件：<xiaowuga@gmail.com>