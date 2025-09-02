#pragma once

#include"../BFC/def.h"
#include"../BFC/bfstream.h"
#include"../BFC/ctc.h"
#include<memory>
#include<map>
#include"opencv2/highgui.hpp"
#include"RudeSocket/socket.h"

_FF_BEG

using cv::Mat;

enum 
{//!!The value should be consistent with netcall.py
	nct_unknown=0,
	nct_char,
	nct_uchar,
	nct_short,
	nct_ushort,
	nct_int32,
	nct_int64,
	nct_float,
	nct_double,
	nct_string,
	nct_image,
	nct_file,
	nct_list,
};


struct nct
{
	class Image
		:public cv::Mat
	{
	public:
		std::string ext;
	public:
		Image(const cv::Mat &img, const std::string &_ext = ".jpg")
			:cv::Mat(img), ext(_ext)
		{
		}
	};
	class File
	{
	public:
		std::string name;
		std::shared_ptr<std::string> data;
	public:
		template<typename _OpT>
		typename std::result_of<_OpT(const std::string&)>::type decode(_OpT decodeOp, std::string file="")
		{
			if (!data || data->empty())
				FF_EXCEPTION1("file open failed");

			if(file.empty())
				file = ff::GetFileName(name);

			FILE* fp = fopen(file.c_str(), "wb");
			if (!fp || fwrite(&(*data)[0], 1, data->size(), fp) != data->size())
				throw file;
			fclose(fp);
			return decodeOp(file);
		}
	};
public:
	static int32 nct_to_cv(int32 type)
	{
		CV_Assert(type != nct_int64); //int64 currently is not supported 

		return type == nct_char ? CV_8S :
			type == nct_uchar ? CV_8U :
			type == nct_short ? CV_16S :
			type == nct_ushort ? CV_16U :
			type == nct_int32 ? CV_32S :
			type == nct_float ? CV_32F :
			type == nct_double ? CV_64F :
			0;
	}
	static int32 cv_to_nct(int32 cvt)
	{
		return cvt == CV_8S ? nct_char :
			cvt == CV_8U ? nct_uchar :
			cvt == CV_16S ? nct_short :
			cvt == CV_16U ? nct_ushort :
			cvt == CV_32S ? nct_int32 :
			cvt == CV_32F ? nct_float :
			cvt == CV_64F ? nct_double :
			nct_unknown;
	}
private:
	static int32 _get_nct(std::string) { return nct_string; }
	static int32 _get_nct(char) { return nct_char; }
	static int32 _get_nct(uchar) { return nct_uchar; }
	static int32 _get_nct(short) { return nct_short; }
	static int32 _get_nct(ushort) { return nct_ushort; }
	static int32 _get_nct(int) { return nct_int32; }
	static int32 _get_nct(long long) { return nct_int64; }
	static int32 _get_nct(float) { return nct_float; }
	static int32 _get_nct(double) { return nct_double; }
	/*template<typename _ValT>
	static int32 _get_nct(const std::vector<_ValT>&)
	{ 
		auto baset=_get_nct(_ValT()); 
		CV_Assert(is_base_type(baset));
		return make_nct(baset, 1);
	}*/
	static int32 _get_nct(const cv::Mat &m)
	{
		//int dim = m.channels() == 1 ? 2 : 3;
		//return make(cv_to_nct(m.depth()), dim);
		return cv_to_nct(m.depth());
	}

public:
	template<typename _ValT>
	static int32 get(const _ValT &v, bool assertKnown=true)
	{
		int32 t=_get_nct(v);
		if (assertKnown)
			CV_Assert(t != nct_unknown);
		return t;
	}
};

class ObjStream
{
	std::shared_ptr<BMStream>  _stream=std::shared_ptr<BMStream>(new BMStream);


	static std::vector<int> _getArrayShape(BMStream &stream, int elemSize)
	{
		//int dim = nct::get_dim(type);
		//CV_Assert(uint(dim) <= 4);
		const int MAX_DIM = 4;

		int n = 1;
		int32 m;
		std::vector<int32> shape;
		for (int i = 0; i <= MAX_DIM; ++i)
		{
			stream >> m;
			if (m <= 0)
				break;
			shape.push_back(m);
			n *= m;
		}
		CV_Assert(m <= 0);
		/*size_t totalSize = sizeof(int32)*(shape.size()+1) + n*elemSize;
		size_t streamSize = stream.Size();
		if ( totalSize!= streamSize)
			throw std::exception("invalid data size");*/
		return shape;
	}
	
	template<typename _ValT>
	void _store(const _ValT &obj)
	{
		this->_stream->Clear();
		_put(obj);
	}
public:
	
public:
	ObjStream()
	{}
	template<typename _ValT>
	ObjStream(const _ValT &obj)
	{
		_store(obj);
	}
	ObjStream(const char *str)
	{
		_store(std::string(str));
	}
	
	BMStream& stream() const
	{
		return *_stream;
	}
	
	template<typename _ValT>
	_ValT get()
	{
		CV_Assert(!_stream->Empty());

		_ValT obj;
		_stream->Seek(0, SEEK_SET);
		_get(obj);
		return obj;
	}
	template<typename _ValT>
	std::vector<_ValT> getv()
	{
		return this->get<std::vector<_ValT>>();
	}
	cv::Mat getm()
	{
		return this->get<cv::Mat>();
	}
	int size() const
	{
		return _stream->Size();
	}
	const char* data() const
	{
		return _stream->Buffer();
	}
	/*void setBytes(const void *bytes, int size)
	{
		_stream->Clear();
		_stream->Write(bytes, size, 1);
	}*/

	template<typename _Stream>
	friend void BFSWrite(_Stream &os, const ObjStream &obj)
	{
		os << (int32)obj.size();
		os.Write(obj.data(), obj.size(), 1);
	}

	template<typename _Stream>
	friend void BFSRead(_Stream &is, ObjStream &obj)
	{
		int32 dsize;
		is >> dsize;
		auto stream = std::shared_ptr<BMStream>(new BMStream);
		stream->Clear();
		stream->Resize(dsize);
		is.Read((char*)stream->Buffer(), dsize, 1);
		obj._stream = stream;
	}
private:
	//return short==types of fixed size
	static short is_supported_type(char);
	static short is_supported_type(uchar);
	static short is_supported_type(short);
	static short is_supported_type(ushort);
	static short is_supported_type(int);
	static short is_supported_type(uint);
	static short is_supported_type(int64);
	static short is_supported_type(uint64);
	static short is_supported_type(float);
	static short is_supported_type(double);
	//template<typename _ValT>
	//static short is_supported_type(cv::Point_<_ValT>);
	//return int ==types of unknown encoded size
	/*static int is_supported_type(std::string);
	template<typename _ValT>
	static int is_supported_type(std::vector<_ValT>);*/
	//return char == unsurpported types
	static char is_supported_type(...);

	struct _ObjHead
	{
		int32 type = nct_unknown;
	};

	template<typename _OpT>
	void _put_x(_OpT op)
	{
		static_assert(sizeof(_ObjHead) == 4,"");
		_ObjHead head;
		_stream->Write(&head, sizeof(head), 1);
		BMStream::PosType pos = _stream->Tell();
		op(head);
		//head.size = int32(_stream->Tell() - pos);
		_stream->WriteAt(&head, pos - sizeof(head), sizeof(head), 1);
	}
	template<typename _OpT>
	void _get_x(_OpT op)
	{
		static_assert(sizeof(_ObjHead) == 4,"");
		_ObjHead head;
		_stream->Read(&head, sizeof(head), 1);
		op(head);
	}
	template<typename _ValT>
	void _put(const _ValT &val)
	{
		this->_put_x([&val, this](_ObjHead &head) {
			static_assert(sizeof(is_supported_type(_ValT()))>1, "unsurpported type");
			(*_stream).Write(&val, sizeof(val), 1);
			head.type=nct::get(val);
		});
	}
	template<typename _ValT>
	void _get(_ValT &val)
	{
		this->_get_x([&val, this](const _ObjHead &head) {
			CV_Assert(head.type == nct::get(val));
			static_assert(sizeof(is_supported_type(_ValT())) > 1, "unsurpported type");
			(*_stream).Read(&val, sizeof(val), 1);
		});
	}
	void _put(const std::string &val)
	{
		this->_put_x([&val, this](_ObjHead &head) {
			(*_stream) << uint32(val.size());
			(*_stream).Write(val.data(), val.size(), 1);
			head.type=nct::get(val);
		});
	}
	void _get(std::string &val)
	{
		this->_get_x([&val, this](const _ObjHead &head) {
			CV_Assert(head.type == nct_string);
			uint32 size;// = _stream->Size();
			(*_stream) >> size;
			val.resize(size);
			if (size>0)
				_stream->Read(&val[0], val.size(), 1);
		});
	}

	void _get(nct::File &val)
	{
		this->_get_x([&val, this](const _ObjHead &head) {
			CV_Assert(head.type == nct_file);
			uint32 size;// = _stream->Size();
			(*_stream) >> size;
			val.name.resize(size);
			if (size>0)
				_stream->Read(&val.name[0], size, 1);

			auto data = std::make_shared<std::string>();
			(*_stream) >> size;
			data->resize(size);
			if (size>0)
				_stream->Read(&(*data)[0], size, 1);
			val.data = data;
		});
	}

	int _compact_shape(int shape[], int n)
	{
		int beg = 0, end = n;
		for (; beg<n && shape[beg] <= 1; ++beg);
		for (; end > 0 && shape[end - 1] <= 1; --end);
		if (end <= beg)
			shape[0] = 1, n = 1;
		else if (beg > 0)
		{
			for (int i = beg; i < end; ++i)
				shape[i - beg] = shape[i];
			n = end - beg;
		}
		return n;
	}
	void _put(const Mat &m, bool compactShape=false)
	{
		this->_put_x([&m, compactShape, this](_ObjHead &head) {
			int32 shape[] = { m.rows,m.cols,m.channels() };
			int n = 3;
			if (compactShape)
			{
				n = _compact_shape(shape, 3);
				CV_Assert(n >= 1);
			}
			for (int i = 0; i < n; ++i)
				(*_stream) << shape[i];
			(*_stream)<< int32(-1);
			for (int i = 0; i < m.rows; ++i)
				_stream->Write(m.ptr(i), m.elemSize()*m.cols, 1);
			head.type=nct::cv_to_nct(m.depth());
		});
	}

	void _put(const nct::Image &m)
	{
		this->_put_x([&m, this](_ObjHead &head) {
			std::vector<uchar> buf;
			if (!cv::imencode(m.ext, m, buf))
				throw std::exception("imencode failed");
			
			(*_stream) << (uint32)buf.size();
			if (!buf.empty())
				_stream->Write(&buf[0], buf.size(), 1);
			head.type = nct_image;
		});
	}

	Mat _getAsImage()
	{
		uint32 size;
		(*_stream) >> size;
		std::vector<uchar> buf(size);
		_stream->Read(&buf[0], size, 1);
		return cv::imdecode(buf, cv::IMREAD_UNCHANGED);
	}
	Mat  _getAsMat(int32 type)
	{
		Mat m;
		int depth = nct::nct_to_cv(type);
		int elemSize = CV_ELEM_SIZE1(depth);
		std::vector<int> shape = this->_getArrayShape(*_stream, elemSize);

		int nelems = 1;
		for (auto v : shape)
			nelems *= v;
		if (nelems <= 0)
			return m;

		while (!shape.empty() && shape.back() == 1)
			shape.pop_back();

		int dims = (int)shape.size();
		if (dims <= 2 || (dims == 3 && shape[2] <= CV_CN_MAX))
		{
			int rows = dims >= 1 ? shape[0] : 1, cols = dims >= 2 ? shape[1] : 1, channels = dims >= 3 ? shape[2] : 1;
			int type = CV_MAKETYPE(depth, channels);

			m.create(rows, cols, type);
			CV_Assert(m.elemSize()*m.cols == m.step);
		}
		else
		{
			int type = CV_MAKETYPE(depth, 1);
			m.create(dims, &shape[0], type);
		}

		_stream->Read(m.data, elemSize*nelems, 1);
		return m;
	}
	void _get(Mat &m)
	{
		this->_get_x([&m, this](const _ObjHead &head) {
			if (head.type == nct_image)
				m = this->_getAsImage();
			else
			{
				m = this->_getAsMat(head.type);
			}
		});
	}


	template<typename _PixT>
	void _put(const cv::Mat_<_PixT> &m)
	{
		_put((const Mat&)m);
	}
	template<typename _PixT>
	void _get(cv::Mat_<_PixT> &m)
	{
		_get((Mat&)m);
	}
	template<typename _ValT, int m, int n>
	void _put(const cv::Matx<_ValT, m, n> &v)
	{
		_put(Mat(v));
	}
	template<typename _ValT, int m, int n>
	void _get(cv::Matx<_ValT, m, n> &v)
	{
		Mat t;
		_get(t);
		v = t;
	}


	template<typename _ListT>
	void _put_list(const _ListT &vlist)
	{
		this->_put_x([&vlist, this](_ObjHead &head) {
			(*_stream) << (uint)vlist.size();
			for (auto &v : vlist)
				this->_put(v);
			head.type = nct_list;
		});
	}
	template<typename _ListT>
	void _get_list(_ListT &vlist)
	{
		this->_get_x([&vlist, this](const _ObjHead &head) {
			CV_Assert(head.type == nct_list);
			uint size;
			(*_stream) >> size;
			vlist.resize(size);
			for (auto &v : vlist)
				_get(v);
		});
	}

	template<typename _ValT>
	void _put(const std::vector<_ValT> &v)
	{
		cv::InputArray in(v);
		_put(in.getMat(),true);
	}
	void _put(const std::vector<std::string> &v)
	{
		_put_list(v);
	}
	template<typename _ValT>
	void _put(const std::vector<std::vector<_ValT>> &v)
	{
		_put_list(v);
	}
	void _put(const std::vector<Mat> &v)
	{
		_put_list(v);
	}

	template<typename _ValT>
	void _get(std::vector<_ValT> &v)
	{
		Mat m;
		this->_get(m);

		CV_Assert(m.step == m.elemSize()*m.cols);
		int size = m.elemSize()*m.rows*m.cols;
		CV_Assert(size % sizeof(_ValT) == 0);
		size /= sizeof(_ValT);

		v.resize(size);
		if (size > 0)
			memcpy(&v[0], m.data, size * sizeof(_ValT));
	}
	void _get(std::vector<std::string> &v)
	{
		_get_list(v);
	}
	template<typename _ValT>
	void _get(std::vector<std::vector<_ValT>> &v)
	{
		_get_list(v);
	}
	void _get(std::vector<Mat> &v)
	{
		_get_list(v);
	}
	

#if 0
	template<typename _ValT>
	friend ObjStream& operator << (ObjStream &os, const std::vector<_ValT> &v)
	{
		bool withElemSize = sizeof(is_supported_type(_ValT()))!=sizeof(short);
		os << (int32)v.size();
		if (withElemSize)
		{
			for (auto &x : v)
			{
				long p = os._stream->Tell();
				os << (int32)0 << x;
				
				int32 xsize = os._stream->Tell() - p - sizeof(int32);
				os.stream().WriteAt(&xsize, p, sizeof(int32), 1);
			}
		}
		else
		{
			if(!v.empty())
				os._stream->Write(&v[0], sizeof(_ValT), v.size());
		}
		return os;
	}
	template<typename _ValT>
	friend ObjStream& operator >> (ObjStream &is, std::vector<_ValT> &v)
	{
		bool withElemSize = sizeof(is_supported_type(_ValT())) != sizeof(short);
		int32 size;
		is >> size;
		v.resize(size);

		if (withElemSize)
		{
			char *buf = (char*)is.stream().Buffer();
			long p = sizeof(int32);
			long dsize = is._stream->Size();

			ObjStream ts;
			for (int i = 0; i < v.size(); ++i)
			{
				is._stream->ReadAt(&size, p, sizeof(int32), 1);
				p += sizeof(int32);
				ts._stream->SetBuffer(buf + p, size, false);
				ts >> v[i];
				p += size;
			}
		}
		else
		{
			FFAssert(is._stream->Size() >= sizeof(_ValT)*size);
			is._stream->Read(&v[0], sizeof(_ValT), size);
		}
		return is;
	}

	template<typename _ValT, int _N>
	friend ObjStream& operator<<(ObjStream &os, const _ValT(&arr)[_N])
	{
		(*os._stream) << _N << arr;
		return os;
	}
	template<typename _ValT, int _N>
	friend ObjStream& operator >> (ObjStream &is, _ValT(&arr)[_N])
	{
		is.getNDArray<_ValT>([&arr](const std::vector<int> &shape) {
			FFAssert(shape[0] == _N);
			return arr;
		});
		return is;
	}
	template<typename _ValT, int _M, int _N>
	friend ObjStream& operator<<(ObjStream &os, const _ValT(&arr)[_M][_N])
	{
		(*os._stream) << _M << _N << arr;
		return os;
	}
	template<typename _ValT, int _M, int _N>
	friend ObjStream& operator>>(ObjStream &is, const _ValT(&arr)[_M][_N])
	{
		is.getNDArray<_ValT>([&arr](const std::vector<int> &shape) {
			FFAssert(shape[0] == _M && shape[1]==_N);
			return arr;
		});
		return is;
	}
	friend ObjStream& operator<<(ObjStream &os, const cv::Mat &m)
	{
		os << m.rows << m.cols << m.channels();
		for (int i = 0; i < m.rows; ++i)
			os.stream().Write(m.ptr(i), m.elemSize()*m.cols, 1);
		return os;
	}
	template<typename _PixelT>
	friend ObjStream& operator << (ObjStream &os, const cv::Mat_<_PixelT> &m)
	{
		os << (const cv::Mat&)m;
		return os;
	}
	template<typename _PixelT>
	friend ObjStream& operator >> (ObjStream &is, cv::Mat_<_PixelT> &m)
	{
		typedef typename cv::Mat_<_PixelT>::channel_type _ValT;
		Mat t = is.getMat<_ValT>();
	//	FFAssert(t.dims >= 3 && m.channels() == 1 || t.dims == 2 && (m.channels() == t.channels()||m.channels()==1));
		m = t;
		return is;
	}
	template<typename _ValT,int m,int n>
	friend ObjStream& operator<<(ObjStream &stream, const cv::Matx<_ValT, m, n> &v)
	{
		stream.stream() << m << n;
		stream.stream().Write(v.val, sizeof(_ValT)*m*n, 1);
		return stream;
	}
	template<typename _ValT, int m, int n>
	friend ObjStream& operator>>(ObjStream &stream, cv::Matx<_ValT, m, n> &v)
	{
		stream.getNDArray<_ValT>([&v](const std::vector<int> &shape) {
			FFAssert(shape.size()>=2 && shape[0] == m && shape[1] == n);
			return v.val;
		});
		return stream;
	}
#endif
};

//typedef std::map<std::string, ObjStream> NetObjs;

class NetObjs
	:public std::map<std::string, ObjStream>
{
	typedef std::map<std::string, ObjStream> _BaseT;
public:
	using _BaseT::_BaseT;

	//operator bool() 
	bool hasError()
	{
		auto itr = this->find("error");// != this->end()
		if (itr == this->end())
			return false;
		return itr->second.get<int>() < 0;
	}
	bool operator!()
	{
		return !hasError();
	}
};

inline std::string netcall_encode(const NetObjs &objs)
{
	OBMStream os;
	os << int32(0);

	for (auto &v : objs)
	{
		os << v.first << v.second;
	}
	int32 totalSize = os.Size() - sizeof(int32);
	os.Seek(0, SEEK_SET);
	os << totalSize;

	const char *data = os.Buffer();
	return std::string(data, data + os.Size());
}

inline NetObjs netcall_decode(const std::string &data, bool skipHead=true)
{
	IBMStream is;
	is.SetBuffer((char*)data.data(), data.size(), false);

	if(skipHead)
		is.Seek(sizeof(int32), SEEK_SET);

	NetObjs objs;

	std::string name;
	ObjStream obj;
	while (is.Tell() < is.Size())
	{
		is >> name >> obj;
		objs.emplace(name, obj);
	}
	return objs;
}

inline std::string netcall_recv(rude::Socket *net)
{
	auto readData = [net](void *buf, int size) {
		int r = net->read((char*)buf, size);
		if (r != size)
			FF_EXCEPTION1("net read failed");
		return r;
	};

	int32 size;
	readData(&size, sizeof(size));

	std::string data;
	data.resize(sizeof(size) + size);
	readData(&data[sizeof(size)], size);
	memcpy(&data[0], &size, sizeof(size));

	return data;
}

class _BFC_API NetcallServer
	:public rude::Socket
{
	bool _connected = false;
public:
	NetcallServer(){}

	NetcallServer(const char *server, int port)
	{
		this->connect(server, port);
	}
	~NetcallServer()
	{
		if (_connected)
			this->sendExit();
	}

	void connect(const char *server, int port)
	{
		if (_connected)
			this->sendExit();

		if (!this->rude::Socket::connect(server, port))
			FF_EXCEPTION1("failed to connect with netcall server");
		_connected = true;
	}

	NetObjs  call(const NetObjs &objs, bool recv=true)
	{
		auto data = netcall_encode(objs);
		int r=this->send(data.data(), data.size());
		if (r != data.size())
			FF_EXCEPTION1(ff::StrFormat("netcall send failed (return %d, expected %d)", r, (int)data.size()));
		
		if (!recv)
			return NetObjs();

		auto rdata = netcall_recv(this);
		return netcall_decode(rdata);
	}
	void sendExit()
	{
		ff::NetObjs objs = {
			{ "cmd","exit" }
		};
		this->call(objs, false);
		_connected = false;
	}
};


_FF_END


