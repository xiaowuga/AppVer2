#pragma once

#include"BasicData.h"
#include"ARModule.h"
// #include"RPC.h"
#include<thread>

class ARApp
{
public:
	std::string  name;
	AppDataPtr  appData;
	SceneDataPtr  sceneData;
    FrameDataPtr  frameData;
    std::shared_mutex  dataMutex;
	std::vector<ARModulePtr> modules;
	std::shared_ptr<RPCClientConnection>  rpcConnection;
private:
    int    _maxBufferedPostedFrames = 2;

public:
    ARApp() = default;

	int init(const std::string &name, AppDataPtr appData, SceneDataPtr sceneData, const std::vector<ARModulePtr>& modules);

	void connectServer(const std::string& ip, int port);

	void postRemoteCall(RemoteProcPtr  proc)
	{
		if (rpcConnection)
			rpcConnection->post(proc);
	}
	void postRemoteCall(ARModule* modulePtr, FrameDataPtr frameDataPtr, SerilizedObjs& send, RPCFlags flags=RPCFlags(0))
	{
		this->postRemoteCall(
			std::make_shared<RemoteProc>(modulePtr,frameDataPtr, send, flags)
		);
	}

    /*����RPC���Ͷ����л���֡������������Ա��ⷢ�Ͷ�������
    * ������Ͷ����еĻ���֡��������maxBufferedPostedFrames-1����ǰ֡��CollectRemoteProcs�����ᱻ���ã�����������Ҳ���ᱻִ��
    * ���maxBufferedPostedFrames<=0����ÿһ֡���ݶ������ϴ��������粻�ѻ���֡Ƶ�ϸ�ʱ�ᵼ������������ʱ��
    * Ĭ��ֵmaxBufferedPostedFrames==2
    */
    void setMaxBufferedPostedFrames(int maxBufferedPostedFrames)
    {
        _maxBufferedPostedFrames = maxBufferedPostedFrames;
    }

	void run(bool grabRequired=false);


    void start();

    void stop();

    class IGrabFunctor
    {
    public:
        virtual IGrabFunctor* clone() =0;

        virtual void grab(ARApp *app,  FrameData *frameData) =0;

        template<typename _DataT>
        _DataT* getData()
        {
            return (_DataT*)this->_getData();
        }

        virtual void* _getData() =0;

        virtual ~IGrabFunctor(){};
    };

    template<typename _DataT, typename _GrabOpT>
    class GrabFunctor
            :public IGrabFunctor
    {
    public:
        _DataT  data;
        _GrabOpT grabOp;
    public:
        GrabFunctor(const _GrabOpT &_grabOp=_GrabOpT())
                :grabOp(_grabOp)
        {}
        virtual IGrabFunctor* clone()
        {
            return new GrabFunctor<_DataT,_GrabOpT>(grabOp);
        }

        virtual void grab(ARApp *app, FrameData *frameData)
        {
            data=grabOp(app, frameData);
        }
        virtual void* _getData()
        {
            return &data;
        }
    };

    int addGrabFunctor(std::shared_ptr<IGrabFunctor> fptr);

    template<typename _GrabOpT>
    int addGrabFunctorT(_GrabOpT op)
    {
        typedef decltype(op(nullptr)) _DataT;
        return this->addGrabFunctor(std::shared_ptr<IGrabFunctor>(new GrabFunctor<_DataT, _GrabOpT>(op)));
    }

    struct GrabbedData
    {
        std::vector<std::shared_ptr<IGrabFunctor>> datas;
    };

    GrabbedData getGrabbed();

    std::shared_ptr<IGrabFunctor> getGrabbed(int id);

    template<typename _DataT>
    std::tuple<bool, _DataT> getGrabbedT(int id)
    {
        auto fptr=this->getGrabbed(id);
        if(!fptr)
            return std::make_tuple(false,_DataT());

        return std::make_tuple(true, *fptr->getData<_DataT>());
    }

    int call(ARModule *module, int cmd, std::any &data);

    ARModulePtr  getModule(const std::string &name);

    int call(const std::string &moduleName, int cmd, std::any &data)
    {
        return this->call(this->getModule(moduleName).get(), cmd, data);
    }
    void run_step()
    {
        FrameDataPtr frameData = std::make_shared<FrameData>();
        for (size_t i = 0; i < modules.size(); ++i)
        {
            if (modules[i]->Update(*appData, *sceneData, frameData) != STATE_OK)
                return;
        }
    }
private:
    std::vector<std::shared_ptr<IGrabFunctor>> _grabFunctors;
    std::shared_ptr<std::thread> _runThread=nullptr;

    std::mutex          _grabMutex;
    GrabbedData         _grabbedData;
};

