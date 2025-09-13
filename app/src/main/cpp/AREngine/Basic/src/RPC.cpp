
#include "RPC.h"
#include "opencv2/highgui.hpp"
using namespace cv;
#include "App.h"

#include <thread>
#include <atomic>
#include "thread.h"

static auto  _tbeg = std::chrono::system_clock::now();
using namespace std;
using namespace chrono;

double elapsed()
{
	auto now = std::chrono::system_clock::now();
	auto duration = duration_cast<microseconds>(now - _tbeg);
	double t = (double(duration.count()) / microseconds::period::den);
	return t;
}

void Image::_decode(const std::vector<uchar> &buf, Image &v)
{
	(Mat &)v = cv::imdecode(buf, cv::IMREAD_UNCHANGED);
}

void Image::_encode(std::vector<uchar> &buf, const Image &v)
{
	auto &ext = v.ext;
	if (!cv::imencode(ext.empty() ? ".jpg" : ext, (cv::Mat)v, buf))
		throw std::runtime_error("failed to encode image");
}

// Bytes Bytes::encodeImage(const cv::Mat &img, const std::string &ext)
//{
//	std::vector<uchar> buf;
//	if (!cv::imencode(ext, img, buf))
//		throw std::runtime_error("failed to encode image");
//
//	return Bytes(buf);
// }
//
// cv::Mat Bytes::getImage() const
//{
//	auto buf = this->get<std::vector<uchar>>();
//	Mat img = cv::imdecode(buf, cv::IMREAD_UNCHANGED);
//	return img;
// }

Image _encodeDepth(const Mat& m)
{
	CV_Assert(m.type() == CV_16UC1 || m.type() == CV_32FC1);
	Mat dm(m.size(), m.type() == CV_16UC1 ? CV_8UC3 : CV_8UC4);

	for (int y = 0; y < m.rows; ++y)
	{
		memcpy(dm.ptr(y), m.ptr(y), m.elemSize() * m.cols);
	}

	return Image(dm, ".png");
}

Mat _decodeDepth(const Mat& m)
{
	CV_Assert(m.type() == CV_8UC3 || m.type() == CV_8UC4);
	Mat dm(m.size(), m.type() == CV_8UC3 ? CV_16UC1 : CV_32FC1);

	for (int y = 0; y < m.rows; ++y)
	{
		memcpy(dm.ptr(y), m.ptr(y), dm.elemSize() * dm.cols);
	}
	return dm;
}

void SerilizedFrame::encode(SerilizedObjs &sendObjs, const FrameData &frameData) const
{
	for (auto &v : (*this))
	{
		if (strncmp(v.first.c_str(), "RGB", 3) == 0)
		{
			int idx;
			if (sscanf(v.first.c_str() + 3, "%d", &idx) == 1 && uint(idx) < frameData.image.size())
			{
				std::string ext = (*this).getd<std::string>(v.first, ".jpg");

				sendObjs[v.first] = Image(frameData.image[idx], ext);
			}
		}

		if (strncmp(v.first.c_str(), "Depth", 5) == 0)
		{
			int idx;
			if (sscanf(v.first.c_str() + 5, "%d", &idx) == 1 && uint(idx) < frameData.depth.size())
			{
				sendObjs[v.first] = _encodeDepth(frameData.depth[idx]);
			}
		}
	}
	sendObjs["frameID"] = frameData.frameID;

	// return SerilizedObjs::encode(objs);
}
void SerilizedFrame::decode(SerilizedObjs &sendObjs, FrameData &frameData)
{
	frameData.image.clear();
	frameData.depth.clear();

	for (auto &v : sendObjs)
	{
		if (strncmp(v.first.c_str(), "RGB", 3) == 0)
		{
			int idx;
			if (sscanf(v.first.c_str() + 3, "%d", &idx) == 1)
			{
				CV_Assert(uint(idx) < 5);
				if (idx >= frameData.image.size())
					frameData.image.resize(idx + 1);

				frameData.image[idx] = v.second.get<Image>();
			}
		}
		if (strncmp(v.first.c_str(), "Depth", 5) == 0)
		{
			int idx;
			if (sscanf(v.first.c_str() + 5, "%d", &idx) == 1)
			{
				CV_Assert(uint(idx) < 5);
				if (idx >= frameData.depth.size())
					frameData.depth.resize(idx + 1);

				frameData.depth[idx] = _decodeDepth(v.second.get<Image>());
			}
		}
	}
	frameData.frameID = sendObjs["frameID"].get<uint>();
}

Bytes RemoteProc::encode(bool sendOrRet)
{
	SerilizedObjs &tar(sendOrRet ? send : ret);
	tar["#procID"] = id; // # for system added objects
	tar["#flags"] = (uint)flags;
	if (sendOrRet)
	{
		if (modulePtr)
		{
			CV_Assert(!modulePtr->getModuleName().empty());
			tar["#module"] = modulePtr->getModuleName();
		}
		else
			tar["#module"] = std::string("");

		tar["#frameID"] = frameDataPtr ? frameDataPtr->frameID : uint(0);
	}
	return tar.encode();
}

std::shared_ptr<RemoteProc> RemoteProc::decode(SerilizedObjs &objs, bool sendOrRet)
{
	auto proc = std::make_shared<RemoteProc>();
	proc->id = objs["#procID"].get<uint>();
	proc->flags = (RPCFlags)objs["#flags"].get<uint>();

	SerilizedObjs &tar(sendOrRet ? proc->send : proc->ret);
	tar.swap(objs);

	return proc;
}

static std::string getCmdName(SerilizedObjs& obj)
{
	return obj.has("cmd") ? obj.get<string>("cmd") : obj.getd<string>("#cmd","unknown");
}

static double CLIENT_PROC_WAIT_TIMEOUT = 30.0; //wait at most 30 seconds

class RPCClientConnectionImpl
	: public RPCClientConnection
{
	ARApp *_app = nullptr;

	struct DProc
	{
		RemoteProcPtr  ptr=nullptr;
		double  timeStamp = 0.0;
		bool    isFrameData = false;
	public:
		RemoteProcPtr operator->() const
		{
			return ptr;
		}
	};

	struct ProcBuffer
	{
		std::list<DProc> que;
		std::mutex mtx;
	};
	ProcBuffer _bufPosted;
	ProcBuffer _bufSended;
	ProcBuffer _bufRecved;
	uint _curProcID = 0;
	std::atomic_int32_t _nBufferedFrames=0;

	ff::Thread _bgThread;
	std::atomic_bool _toExit = false;
	std::vector<std::shared_ptr<std::thread>> _threads;

	void _sendThread()
	{
		while (!_toExit)
		{
			DProc proc = { nullptr };

			{
				std::lock_guard<std::mutex> _lock(_bufPosted.mtx);
				if (!_bufPosted.que.empty())
				{
					proc = _bufPosted.que.front();
					_bufPosted.que.pop_front();
				}
			}

			if (proc.ptr)
			{
				Bytes data = proc->encode(true);

				if (proc->modulePtr) // return handling required
				{
					std::lock_guard<std::mutex> _lock(_bufSended.mtx);
					_bufSended.que.push_back(proc);

					double curTime = elapsed();
					while (!_bufSended.que.empty() && curTime - _bufSended.que.front().timeStamp > CLIENT_PROC_WAIT_TIMEOUT)
						_bufSended.que.pop_front();
				}
				
				this->writeBlock(data); //send data after .push_back(proc) to avoid "find match error" in _recvThread!!
				//std::this_thread::sleep_for(std::chrono::milliseconds(100));

				if (proc.isFrameData)
					_nBufferedFrames -= 1;
			}
			else
			{
				_nBufferedFrames = 0;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}
	void _recvThread()
	{
		while (!_toExit)
		{
			Bytes data = this->readBlock();
			if (!data.empty())
			{
				auto dptr = RemoteProc::decode(data, false);

				//printf("recved: id=%d\n", proc->id);

				{
					std::lock_guard<std::mutex> _lock(_bufSended.mtx);

					auto itr = _bufSended.que.begin();
					for (; itr != _bufSended.que.end(); ++itr)
					{
						if ((*itr)->id == dptr->id)
							break;
					}

					if (itr != _bufSended.que.end())
					{
						(*itr)->ret.swap(dptr->ret);
						DProc proc = *itr;
						_bufSended.que.erase(itr);

						std::lock_guard<std::mutex> _lock(_bufRecved.mtx);
						_bufRecved.que.push_back(proc);
					}
					else
						printf("error : failed to find match for received proc.\n");
				}
			}
		}
	}
	void _proThread()
	{
		while (!_toExit)
		{
			DProc proc = { nullptr };
			{
				std::lock_guard<std::mutex> _lock(_bufRecved.mtx);
				if (!_bufRecved.que.empty())
				{
					proc = _bufRecved.que.front();
					_bufRecved.que.pop_front();
				}
			}
			if (proc.ptr)
			{
				if (proc->modulePtr)
					proc->modulePtr->ProRemoteReturn(proc.ptr);
			}
			else
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

public:
	~RPCClientConnectionImpl()
	{
		this->close();
	}
	virtual void close()
	{
		if (_app)
		{
			_bgThread.waitAll();
			this->waitDone();

			_toExit = true;
			for (auto& t : this->_threads)
				t->join();
			_threads.clear();

			this->ClientConnection::close();
			_app = nullptr;
		}
	}
	void waitDone(double waitTimeOut=5.0)
	{
		double lastTime = elapsed();
		int lastCode = 0;

		while (true)
		{
			bool _break = false;
			
			{
				std::lock(_bufPosted.mtx, /*_bufSended.mtx,*/ _bufRecved.mtx);

				std::lock_guard<std::mutex> _lock1(_bufPosted.mtx, std::adopt_lock);
				//std::lock_guard<std::mutex> _lock2(_bufSended.mtx, std::adopt_lock);
				std::lock_guard<std::mutex> _lock3(_bufRecved.mtx, std::adopt_lock);

				if (_bgThread.nRemainingMessages() == 0 && _bufPosted.que.empty() && _bufRecved.que.empty())
				{
					std::lock_guard<std::mutex> _lock2(_bufSended.mtx);

					int code = 0;
					for (auto& v : _bufSended.que)
						code += v->id;

					if (code == lastCode)
					{
						double dtime = elapsed() - lastTime;
						if (dtime > waitTimeOut || code==0)
						{
							//for (auto v : _bufSended.que)
							//	printf("unhandled id=%d, cmd=%s\n", v->id, getCmdName(v->send).c_str());

							_break = true;
						}
						printf("waiting done...%.2f seconds   \r", dtime);
					}
					else
					{
						lastTime = elapsed();
						lastCode = code;
					}
				}
				else
				{
					lastTime = elapsed();
					lastCode = 0;
				}
			}
			
			if (_break)
				break;
			else
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		printf("\n");
	}
	virtual void connect(const std::string &ip, int port, ARApp *app)
	{
		CV_Assert(!_app); // not resuable

		this->ClientConnection::connect(ip, port);
		_app = app;

		SerilizedObjs cmd = {
			{"#cmd", "appInit"},
			{"appName", _app->name}};
		for (int i = 1; i <= 5; i++)
		{
			SerilizedObjs ret = this->_call(cmd);

			if (ret.getd<int>("code", STATE_ERROR) != STATE_OK)
			{
				if (i == 5)
				{
					this->close();
					throw std::runtime_error("appInit failed");
				}
				else
				{
					printf("no.%d _call appInit fail", i);
					continue;
				}
			}
			else
			{
				printf("_call appInit ok\n");
				break;
			}
		}
		// SerilizedObjs ret = this->_call(cmd);
		// if (ret.getd<int>("code", STATE_ERROR) != STATE_OK)
		// {
		// 	this->close();
		// 	throw std::runtime_error("appInit failed");
		// }

		_threads.push_back(std::make_shared<std::thread>(
			[this]()
			{ this->_sendThread(); }));

		_threads.push_back(std::make_shared<std::thread>(
			[this]()
			{ this->_recvThread(); }));

		_threads.push_back(std::make_shared<std::thread>(
			[this]()
			{ this->_proThread(); }));
	}

	void _post(RemoteProcPtr proc, bool isFrameData=false)
	{
		//printf("__post cmd=%s\n", proc->send.getd<std::string>("cmd", "unknown").c_str());

		++_curProcID;
		proc->id = _curProcID;
		_bufPosted.que.push_back({ proc,elapsed(), isFrameData });
		if (isFrameData)
			_nBufferedFrames += 1;
	}
	virtual void post(RemoteProcPtr proc)
	{
		// post with bgThread to keep order with "sendFrame"
		_bgThread.post([proc, this]()
					   {
						   std::lock_guard<std::mutex> _lock(_bufPosted.mtx);
						   this->_post(proc); });
	}

	virtual void post(const SerilizedFrame &serilizedFrame, FrameDataPtr frameData, const std::vector<RemoteProcPtr> &procs)
	{
		_bgThread.post([=]()
					   {
						   auto frameProc = std::make_shared<RemoteProc>();
						   frameProc->frameDataPtr = frameData;

						   frameProc->send["#cmd"] = "sendFrame";
						   serilizedFrame.encode(frameProc->send, *frameData);

						   {
							   std::lock_guard<std::mutex> _lock(_bufPosted.mtx);
							   this->_post(frameProc, true);

							   for (auto &proc : procs)
								   this->_post(proc);
						   } });
	}
	virtual int  nBufferedPostedFrames()
	{
		return _nBufferedFrames;
	}
};

std::shared_ptr<RPCClientConnection> RPCClientConnection::create()
{
	return std::make_shared<RPCClientConnectionImpl>();
}
