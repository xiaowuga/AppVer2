#pragma once

#include "BasicData.h"
#include "RPC.h"
#include "ARModule.h"
#include "BFC/thread.h"

class ARAppServer;
typedef std::shared_ptr<ARAppServer> ARAppServerPtr;

class ARModuleServer;
typedef std::shared_ptr<ARModuleServer> ARModuleServerPtr;

class RPCServerConnection
    : public ServerConnection
{
public:
    std::string    name;
    ARAppServerPtr app;
public:
    virtual void start(int nworkers) = 0;
    
    static std::shared_ptr<RPCServerConnection> create();  
};

typedef std::shared_ptr<RPCServerConnection> RPCServerConnectionPtr;

class ARAppServer
{
public:
    std::string name;
    AppDataPtr appData;
    SceneDataPtr sceneData;
    std::vector<RPCServerConnectionPtr> rpcConnections;

public:
    void init(const std::string &appName, const std::string &appDir);

    void closeConnection(RPCServerConnection *con);
};

class ARServerManager
{
    ff::Thread _manageThread;

public:
    static ARServerManager &instance();

    template <typename _OpT>
    void postManage(const _OpT &op, bool waitFinish = false)
    {
        _manageThread.post(op, waitFinish);
    }

    virtual void registerModule(ARModuleServerPtr modulePtr) = 0;

    virtual ARModuleServerPtr getModule(const std::string &name) = 0;

    virtual void start(const std::string &appsDir) = 0;

    virtual void setNewConnection(RPCServerConnectionPtr connection) = 0;

    virtual ~ARServerManager() {}
};

class ARModuleServer
    : public ARModuleBase
{
    friend class _RPCServerConnectionImpl;

protected:
    ARAppServerPtr _app;

public:
    // ����һ���µĶ���ʵ����ÿ��App��ÿ�����Ӷ��ɲ�ͬ�ķ��������д���
    virtual ARModuleServerPtr create() = 0;

    // ��ʼ������ʵ��
    virtual int init(RPCServerConnection &con) = 0;

    // ִ�пͻ��˵�Զ�̵�������
    virtual int call(RemoteProcPtr proc, FrameDataPtr frameDataPtr, RPCServerConnection &con) = 0;
};
