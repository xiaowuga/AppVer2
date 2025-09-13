
#include "stdf.h"
#include "RPCServer.h"
#include "portable.h"
#include <thread>
#include <mutex>
#include <atomic>
#include "thread.h"
#include<iostream>

class _RPCServerConnectionImpl 
	: public RPCServerConnection
{
	 using RPCServerConnection::RPCServerConnection;
	
	std::vector<ARModuleServerPtr> _modules;
	std::mutex _moduleMutex;

	ARModuleServerPtr _getModule(const std::string &moduleName)
	{
		if (moduleName.empty())
			return nullptr;

		std::lock_guard<std::mutex> _lock(_moduleMutex);
		ARModuleServerPtr md;
		for (auto &ptr : _modules)
			if (ptr->getModuleName() == moduleName)
			{
				md = ptr;
				break;
			}
		if (!md)
		{
			ARModuleServerPtr tm = ARServerManager::instance().getModule(moduleName);
			if (tm)
			{
				auto ptr = tm->create();
				ptr->setModuleName(tm->getModuleName());
				ptr->_app = this->app;

				if (ptr->init(*this) != STATE_OK)
					printf("warning: server module %s init failed\n", moduleName.c_str());
				md = ptr;
				_modules.push_back(ptr);
			}
		}
		return md;
	}

	std::list<FrameDataPtr> _frameBuffers;
	std::mutex _frameBufferMutex;

	ff::Thread _recvHelper;

	struct DRemoteProc
	{
		std::string		moduleName;
		RemoteProcPtr	proc;
		bool            skipTested = false;
	};
	struct ProcBuffer
	{
		std::list<DRemoteProc> que;
		std::mutex mtx;
	};
	ProcBuffer _procsRecved;
	std::mutex _writeMutex;
	std::vector<std::shared_ptr<std::thread>> _threads;
	std::atomic_bool _toExit = false;

public:
	void _recvThread()
	{
		while (!_toExit)
		{
			Bytes data = this->readBlock();
			if (data.empty())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			else
			{
				auto objsPtr = std::make_shared<SerilizedObjs>(data);
				auto &objs = *objsPtr;

				std::string cmd = objs.getd<std::string>("#cmd");

				//printf("recved: cmd=%s\n", cmd.empty() ? "proc" : cmd.c_str());

				if (cmd == "sendFrame")
				{
					_recvHelper.post([objsPtr, &objs, this]()
									 {
						auto frameDataPtr = std::make_shared<FrameData>();
						SerilizedFrame::decode(objs, *frameDataPtr);

						std::lock_guard<std::mutex> _lock(this->_frameBufferMutex);
						this->_frameBuffers.push_back(frameDataPtr);

						const int nMax = 30;
						if (_frameBuffers.size()>nMax) //remove history frames
						{
							int nCheck = (int)_frameBuffers.size() - nMax;
							auto itr = _frameBuffers.begin();
							for (int i = 0; i < nCheck; ++i)
							{
								auto cur = itr;
								++itr;
								if ((*cur).use_count() == 1) //if not used in other places
								{
									_frameBuffers.erase(cur);
								}
							}
						} });
					//printf("frame buffer size = %d\n", _frameBuffers.size());
				}
				else if (cmd == "close")
				{
					//printf("#close...\n");

					this->_waitProDone();

					ARServerManager::instance().postManage([this]()
														   { app->closeConnection(this); });
				}
				else
				{
					_recvHelper.post([objsPtr, &objs, this]()
									 {
						uint frameID = objs.getd<uint>("#frameID", 0);
						//if (frameID>0)
						{
							auto proc = RemoteProc::decode(*objsPtr, true);

							//*objsPtr is unusable below because swap is used in decode

							if (frameID > 0)
							{
								std::lock_guard<std::mutex> _lock(this->_frameBufferMutex);
								for (auto& ptr : this->_frameBuffers)
									if (ptr->frameID == frameID)
										proc->frameDataPtr = ptr;

								if (!proc->frameDataPtr)
								{
									printf("warning: frame is not found, the remote proc. is ignored\n");
									return;
								}
							}

							{
								auto moduleName= proc->send.getd<std::string>("#module", "");

								std::lock_guard<std::mutex> _lock(this->_procsRecved.mtx);
								_procsRecved.que.push_back({ moduleName,proc, false});
							}
						} });
				}
			}
		}
	}
	void _waitProDone()
	{
		_recvHelper.waitAll();

		while (true)
		{
			bool _break = false;
			{
				std::lock_guard<std::mutex> _lock(this->_procsRecved.mtx);
				if (_recvHelper.nRemainingMessages()==0 && _procsRecved.que.empty())
					_break = true;
			}
			if (_break)
				break;
			else
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	void _proThread()
	{
		while (!_toExit)
		{
			RemoteProcPtr proc(nullptr); 
			std::string procModuleName;

			{
				std::lock_guard<std::mutex> _lock(this->_procsRecved.mtx);
				
				if (!_procsRecved.que.empty())
				{
					auto itr = _procsRecved.que.begin();
					auto iproc = itr->proc;

					if ((iproc->flags & RPCF_SKIP_BUFFERED_FRAMES) && !itr->skipTested && iproc->frameDataPtr && !itr->moduleName.empty())
					{
						std::string curModuleName = itr->moduleName;
						uint lastFrameID = 0;
						for (auto ritr = _procsRecved.que.rbegin(); ritr != _procsRecved.que.rend(); ++ritr)
						{
							if (curModuleName == ritr->moduleName && ritr->proc->frameDataPtr)
							{
								lastFrameID = ritr->proc->frameDataPtr->frameID;
								break;
							}
						}
						//if proc is not for the last frame, removing the procs of the same module for the buffered frames
						if (iproc->frameDataPtr->frameID < lastFrameID)
						{
							for (; itr != _procsRecved.que.end();)
							{
								if (curModuleName == itr->moduleName && itr->proc->frameDataPtr)
								{
									auto fid = itr->proc->frameDataPtr->frameID;
									if (fid < lastFrameID)
									{
										printf("skip proc: module=%s, frame=%d, id=%d\n", curModuleName.c_str(), itr->proc->frameDataPtr->frameID, itr->proc->id);
										auto eitr = itr;
										++itr;
										_procsRecved.que.erase(eitr);
									}
									else 
									{
										if (fid == lastFrameID)
											itr->skipTested = true; //do not skip again
										++itr;
									}
								}
								else
									++itr;
							}
						}
					}
				}

				if(!_procsRecved.que.empty())
				{
					auto itr = _procsRecved.que.begin();
					proc = itr->proc;
					procModuleName = itr->moduleName;
					_procsRecved.que.pop_front();
				}
			}

			if (!proc)
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			else
			{
				auto &send = proc->send;

				ARModuleServerPtr md = this->_getModule(procModuleName);

				if (!md)
					printf("warning: proc module is not found (moduleName=%s)\n", procModuleName.c_str());
				else
				{
					md->call(proc, proc->frameDataPtr, *this);

					{
						std::lock_guard<std::mutex> _lock(_writeMutex);
						Bytes data = proc->encode(false);
						this->writeBlock(data);
					}
				}
			}
		}
	}
	virtual void start(int nworkers)
	{
		_threads.push_back(std::make_shared<std::thread>(
			[this]()
			{ this->_recvThread(); }));

		if (nworkers <= 0)
			nworkers = 1;

		for (int i = 0; i < nworkers; ++i)
		{
			_threads.push_back(std::make_shared<std::thread>(
				[this]()
				{ this->_proThread(); }));
		}
	}
	virtual void close()
	{
		_toExit = true;
		for (auto &t : this->_threads)
			t->join();
		_threads.clear();

		_recvHelper.waitAll();

		this->ServerConnection::close();
	}
};

std::shared_ptr<RPCServerConnection> RPCServerConnection::create()
{
	return std::make_shared<_RPCServerConnectionImpl>();
}

void ARAppServer::init(const std::string &appName, const std::string &appDir)
{
	this->name = appName;

	appData = std::make_shared<AppData>();
	appData->engineDir = ff::getFullPath(appDir + "/AREngine/");
	appData->dataDir = ff::getFullPath(appDir + "/data/");

	sceneData = std::make_shared<SceneData>();
}

void ARAppServer::closeConnection(RPCServerConnection *con)
{
	if (con)
	{
		printf("%s: close connection...\n", con->name.c_str());

		con->close();

		auto itr = rpcConnections.begin();
		for (; itr != rpcConnections.end(); ++itr)
			if (&(**itr) == con)
				break;
		if (itr != rpcConnections.end())
			rpcConnections.erase(itr);
	}
}

class _ARServerManagerImpl
	: public ARServerManager
{
	std::vector<ARAppServerPtr> _apps;
	std::vector<ARModuleServerPtr> _modules;

public:
	virtual void registerModule(ARModuleServerPtr modulePtr)
	{
		if (modulePtr)
			_modules.push_back(modulePtr);
	}
	virtual ARModuleServerPtr getModule(const std::string &name)
	{
		for (auto &m : _modules)
			if (m->getModuleName() == name)
				return m;
		return nullptr;
	}
	virtual void start(const std::string &appsDir)
	{
		std::vector<std::string> subDirs;
		ff::listSubDirectories(appsDir, subDirs);

		for (auto &subDir : subDirs)
		{
			std::string name = ff::GetFileName(subDir, false);

			auto ptr = std::make_shared<ARAppServer>();
			ptr->init(name, appsDir + "/" + subDir);

			_apps.push_back(ptr);
		}
	}
	virtual ARAppServerPtr getApp(const std::string &name)
	{
		for (auto &ptr : _apps)
			if (ptr->name == name)
				return ptr;
		return ARAppServerPtr(nullptr);
	}
	virtual void setNewConnection(RPCServerConnectionPtr con)
	{
		if (con)
		{
			Bytes initData;
			while (initData.empty())
				initData = con->readBlock();

			SerilizedObjs initSend(initData);
			SerilizedObjs initRet;
			int code = STATE_ERROR;
			try
			{
				std::string cmd = initSend.getd<std::string>("#cmd");

				if (cmd == "appInit")
				{
					std::string appName = initSend.getd<std::string>("appName");
					ARAppServerPtr app = this->getApp(appName);
					if (app)
					{
						con->app = app;

						// int nworkers = initSend.getd<int>("workers", 2);
						int nworkers = initSend.getd<int>("workers", 1);
						con->start(nworkers);
						app->rpcConnections.push_back(con);
						code = STATE_OK;
					}
					else
						initRet["error"] = "app(" + appName + ") not found";
				}
				else
					initRet["error"] = "invalid init command";
			}
			catch (...)
			{
			}
			initRet["code"] = code;
			con->writeBlock(initRet.encode());
		}
	}
};

ARServerManager &ARServerManager::instance()
{
	static auto _inst = std::make_shared<_ARServerManagerImpl>();

	return *_inst;
}

