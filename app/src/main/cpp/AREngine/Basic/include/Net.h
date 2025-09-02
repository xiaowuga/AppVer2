#pragma once

#include<string>
#include<memory>

class Bytes;
class SerilizedObjs;

class IConnection
{
public:
	// ����������ȡ��СΪsize�����ݡ����ݲ�����ȴ����롣
	// ����ʵ�ʶ�ȡ���ֽ���
	virtual int read(void* buf, int size) = 0;

	// �������д���СΪsize������
	// ����ʵ��д����ֽ���
	virtual int write(const void* buf, int size) = 0;

	virtual void close() = 0;

	Bytes readBlock();

	bool writeBlock(const Bytes& data);

	// ͬ�����ã�������ϵͳ��ʼ���������ط���Ҫʹ�á�
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


