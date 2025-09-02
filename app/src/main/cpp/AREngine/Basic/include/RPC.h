#pragma once

//#include "Basic/include/BasicData.h"
//#include"Basic/include/Net.h"
//
//#include "Basic/asset/cvf/BFC/bfstream.h"
//#include "Basic/asset/cvf/CVX/bfsio.h"

#include "BasicData.h"
#include"Net.h"

#include "BFC/bfstream.h"
#include "CVX/bfsio.h"

#undef string_t
#include <map>

inline char AllowCopyWithMemory(Pose);

// To represent encoded image.
class Image
	:public cv::Mat
{
	std::string ext;

	static void _decode(const std::vector<uchar>& buf, Image& v);

	static void _encode(std::vector<uchar>& buf, const Image& v);
public:
	using cv::Mat::Mat;

	//ext==".jpg", ".png", etc.  empty for default encoding (.jpg)
	Image(const cv::Mat &img, const std::string &ext_=".jpg") 
		:cv::Mat(img),ext(ext_)
	{}

	template <typename _IBST>
	friend void BSRead(_IBST& ibs, Image& v)
	{
		std::vector<uchar> buf;
		ibs >> buf;
		Image::_decode(buf, v);
	}
	template <typename _OBST>
	friend void BSWrite(_OBST& obs, const Image& v)
	{
		std::vector<uchar> buf;
		Image::_encode(buf, v);
		obs << buf;
	}
};

class Bytes
	: public std::string
{
	typedef std::string _BaseT;

	template <typename _IBST>
	friend void BSRead(_IBST &ibs, Bytes &v)
	{
		ibs >> (_BaseT &)v;
	}
	template <typename _OBST>
	friend void BSWrite(_OBST &obs, const Bytes &v)
	{
		obs << (const _BaseT &)v;
	}

public:
	Bytes() = default;

	template <typename _ValT>
	Bytes(const _ValT &val)
	{
		ff::OBMStream mstream;
		mstream << val;
		uint size = mstream.Size();
		if (size > 0)
		{
			this->resize(size);
			// mstream.Read(&_data[0], size, 1);
			memcpy(&(*this)[0], mstream.Buffer(), size);
		}
	}
	Bytes(const char *str)
		: Bytes(std::string(str))
	{
	}

	template <typename _ValT>
	_ValT get() const
	{
		_ValT val;
		if (!this->empty())
		{
			ff::IBMStream mstream;
			mstream.SetBuffer((void *)&(*this)[0], this->size(), false);
			mstream >> val;
		}
		return val;
	}
	template <typename _ValT>
	std::vector<_ValT> getv() const
	{
		return this->get<std::vector<_ValT>>();
	}
};

class SerilizedObjs
	: public std::map<std::string, Bytes>
{
	typedef std::map<std::string, Bytes> _BaseT;

public:
	using _BaseT::_BaseT;

	bool has(const std::string &key) const
	{
		return this->find(key) != this->end();
	}

	//get, the key must exist
	template <typename _ValT>
	_ValT get(const std::string& key) const
	{
		auto itr = this->find(key);
		if (itr == this->end())
			throw std::runtime_error("val not found");
		return itr->second.get<_ValT>();
	}

	//safe get, return @defaultValue if @key is not exist.
	template <typename _ValT>
	_ValT getd(const std::string &key, const _ValT &defaultValue = _ValT()) const
	{
		auto itr = this->find(key);
		if (itr == this->end())
			return defaultValue;
		return itr->second.get<_ValT>();
	}

	SerilizedObjs(const Bytes &data)
	{
		if (!data.empty())
		{
			ff::IBMStream mstream;
			mstream.SetBuffer((void *)&data[0], data.size(), false);
			mstream >> (_BaseT &)*this;
		}
	}

	Bytes encode() const
	{
		return Bytes((const _BaseT &)*this);
	}
};

class SerilizedFrame
	:public SerilizedObjs
{
public:
	void addRGBImage(const FrameData &frameData, int idx=0, const std::string &ext=".jpg")
	{
		std::string key=cv::format("RGB%d", idx);
		(*this)[key] = Bytes(ext);
	}
	void addDepthImage(const FrameData &frameData, int idx=0)
	{
		std::string key = cv::format("Depth%d", idx);
		(*this)[key] = Bytes();
	}
	void encode(SerilizedObjs &sendObjs, const FrameData &frameData) const;

	static void decode(SerilizedObjs &sendObjs, FrameData &frameData);
};

class ARModule;

enum RPCFlags
{
	/*���һ��RemoteProc��������������м�֡���Ա���������˵���Ϣӵ��
	* ���һ��ģ��M��ĳ��proc�������˸ñ�ǲ���proc->frame���Ƿ�������Ϣ�����е�����һ֡����proc������ʱ����Ϣ��������������ģ��M���ҷ�����һ֡��procs�������Ƴ���
	* ���øñ�ǲ��������ģ�����Ϣ����Ӱ�졣
	* ע���������һ֡��Ҫ���procs����Ӧ��ֻ�ѵ�һ��proc����ΪRPCF_SKIP_BUFFERED_FRAMES��������ܵ���һֻ֡��������Ϣ�����ò������յĴ�������
	*/
	RPCF_SKIP_BUFFERED_FRAMES = 0x01
};

class RemoteProc
	: public BasicData
{
public:
	ARModule *modulePtr = nullptr;
	FrameDataPtr frameDataPtr = nullptr;

	uint id = 0;
	RPCFlags flags = RPCFlags(0);

	SerilizedObjs send;
	SerilizedObjs ret;

public:
	RemoteProc() = default;

	RemoteProc(ARModule *_modulePtr, FrameDataPtr _frameDataPtr, SerilizedObjs &_send, RPCFlags _flags= RPCFlags(0))
		: modulePtr(_modulePtr), frameDataPtr(_frameDataPtr), send(_send), flags(_flags)
	{
	}

	Bytes encode(bool sendOrRet);

	static std::shared_ptr<RemoteProc> decode(SerilizedObjs &objs, bool sendOrRet);

	static std::shared_ptr<RemoteProc> decode(Bytes &data, bool sendOrRet)
	{
		SerilizedObjs objs(data);
		return decode(objs, sendOrRet);
	}
};

typedef std::shared_ptr<RemoteProc> RemoteProcPtr;


class ARApp;

class RPCClientConnection
	: public ClientConnection
{
public:
	static std::shared_ptr<RPCClientConnection> create();

	virtual void connect(const std::string &ip, int port, ARApp *app) = 0;

	virtual void post(RemoteProcPtr proc) = 0;

	virtual void post(const SerilizedFrame &serilizedFrame, FrameDataPtr frameData, const std::vector<RemoteProcPtr> &procs) = 0;

	virtual int  nBufferedPostedFrames() = 0;
};
