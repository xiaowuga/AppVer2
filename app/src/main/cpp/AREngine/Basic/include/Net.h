#pragma once

#include<string>
#include<memory>

class Bytes;
class SerilizedObjs;

class IConnection
{
public:
	// 从输入流读取大小为size的数据。数据不够则等待输入。
	// 返回实际读取的字节数
	virtual int read(void* buf, int size) = 0;

	// 向输出流写入大小为size的数据
	// 返回实际写入的字节数
	virtual int write(const void* buf, int size) = 0;

	virtual void close() = 0;

	Bytes readBlock();

	bool writeBlock(const Bytes& data);

	// 同步调用，仅用于系统初始化，其它地方不要使用。
	SerilizedObjs _call(const SerilizedObjs& send);

	virtual ~IConnection() {}
};


class ClientConnection
	: public IConnection
{
	class MyImpl;
	std::shared_ptr<MyImpl>  _impl;
public:
	ClientConnection();

	void connect(const std::string& ip, int port);

	virtual void close();

	virtual int read(void* buf, int size);

	virtual int write(const void* buf, int size);
};


class ServerConnection
	: public IConnection
{
	class MyImpl;
	std::shared_ptr<MyImpl>  _impl;
public:
	ServerConnection();

	virtual void close();

	virtual int read(void* buf, int size);

	virtual int write(const void* buf, int size);

	friend void runRPCServer(int port);
};


void runRPCServer(int port);


