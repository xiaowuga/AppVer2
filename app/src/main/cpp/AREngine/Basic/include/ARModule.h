//
// Created by 谢康 on 2023/7/18.
//

#ifndef ARENGINE_ARMODULE_H
#define ARENGINE_ARMODULE_H

#include "BasicData.h"
#include "RPC.h"
#include<mutex>

enum {
    STATE_OK = 0,
    STATE_ERROR = -1
};

class ARModuleBase
{
private:
    std::mutex  _mtx;
    friend class ARApp;
    friend class RPCClientConnectionImpl;

#define _ARENGINE_LOCK_MODULE(modulePtr)  std::lock_guard<std::mutex> _lockModule((modulePtr)->_mtx);

protected:
    std::string  _moduleName;
public:
    virtual ~ARModuleBase() = default;

    //获取唯一的模块名称，在RPC中将被用于与服务器模块配对
    const std::string& getModuleName() {
        return _moduleName;
    }
    void setModuleName(const std::string& name) {
        _moduleName = name;
    }
};

template<typename _ModuleT>
inline std::shared_ptr<_ModuleT> createModule(const std::string& name)
{
    auto ptr = std::make_shared<_ModuleT>();
    ptr->setModuleName(name);
    return ptr;
}

template<typename _ModuleT, typename _CreateFuncT>
inline std::shared_ptr<_ModuleT> createModule(const std::string& name, _CreateFuncT create)
{
    auto ptr = create();
    ptr->setModuleName(name);
    return ptr;
}

enum
{
    MODULE_CMD_BEG=100 //command id less than this value is reserved for system
};

class ARApp;

class ARModule 
    :public ARModuleBase
{
public:
    ARApp* app;
public:
    /**
     * PreCompute 预处理函数
     * 由应用开发引擎调用，进行相应的预处理
     *
     * @param configPath 配置地址
     * @example
     * @return void
     */
    virtual void PreCompute(std::string configPath) {};

    /**
     * Init 初始化函数
     * 初始化算法所需的配置环境，一般只调用一次
     *
     * @param appData 一些全局配置文件，如资源位置等
     * @param frameData 每一帧刷新的数据，主要是传感器获取的数据
     * @param sceneData 场景管理数据，接受算法的输出，作为另一些算法的输入
     * @attention appData与sceneData为全局变量，每次做更新
     * @attention frameData每帧重新生成
     * @example
     * @return void
     */
    virtual int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) {
        return STATE_OK;
    };

    /**
     * Update 更新函数
     * 算法运行中的每帧刷新函数
     *
     * @param appData 一些全局配置文件，如资源位置等
     * @param frameData 每一帧刷新的数据，主要是传感器获取的数据
     * @param sceneData 场景管理数据，接受算法的输出，作为另一些算法的输入
     * @attention appData与sceneData为全局变量，每次做更新
     * @attention frameData每帧重新生成
     * @example
     * @return void
     */
    virtual int Update(AppData &appData, SceneData &sceneData, FrameDataPtr frameDataPtr)=0;

    virtual int ShutDown(AppData& appData,  SceneData& sceneData){
        return STATE_OK;
    }

    /*搜集远程过程调用（RPC）所需的帧数据和命令
    * 对输入模块不会执行该操作，其它模块的该操作在帧数据输入完成之后（Update之前）执行。
    * 函数在主线程中执行，默认该函数的耗时非常短。
    */
    virtual int CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, FrameDataPtr frameDataPtr) {
        return STATE_OK;
    }
    
    /*处理RPC的返回结果。
    * 注意该函数将在子线程中执行，注意与其它函数的线程同步
    */
    virtual int ProRemoteReturn(RemoteProcPtr proc) {
        return STATE_OK;
    }

    /*处理本地的外部调用命令，在主线程中执行
     * cmd是命令ID，
     * data是命令的输入参数或者输出结果
     */
    virtual int ProCommand(int cmd, std::any &data) {
        return STATE_OK;
    }
    
};

typedef std::shared_ptr<ARModule>  ARModulePtr;

//
//class ARModuleEx
//    :public ARModuleBase
//{
//public:
//    ARApp* app;
//public:
//    /**
//     * Init 初始化函数
//     * 初始化算法模块，只在第一帧输入时调用
//     */
//    virtual int Init(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) = 0;
//
//    /**
//     * Update 更新函数
//     * 算法运行中每一帧的更新函数，对场景数据和视频帧数据进行更新
//     */
//    virtual int Update(AppData& appData, SceneData& sceneData, FrameDataPtr frameDataPtr) = 0;
//
//    /**
//    * Destory清除函数，程序退出前调用
//    */
//    virtual int Destory(AppData& appData, SceneData& sceneData) = 0;
//
//    /*搜集远程过程调用（RPC）所需的帧数据和命令
//    * 对输入模块不会执行该操作，其它模块的该操作在帧数据输入完成之后（Update之前）执行。
//    */
//    virtual int CollectRemoteProcs(SerilizedFrame& serilizedFrame, std::vector<RemoteProcPtr>& procs, 
//                                    FrameDataPtr frameDataPtr) = 0;
//
//    /*处理RPC的返回结果。
//    * 注意该函数将在子线程中执行，注意与其它函数的线程同步
//    */
//    virtual int ProRemoteReturn(RemoteProcPtr proc) = 0;
//};


#endif //ARENGINE_ALGORITHM_H
