
#include"Net.h"
#include"RPC.h"
#include"RPCServer.h"
#include<iostream>
using namespace std;

#define USE_SOCKPP  1


Bytes IConnection::readBlock()
{
	Bytes data;
	uint32_t size;
	if (this->read(&size, sizeof(size)) == sizeof(size))
	{
		if (size > 0)
		{
			data.resize(size);
			if (this->read(&data[0], size) != size)
			{
				data.clear();
			}
		}

		uint32_t checkVal;
		if (this->read(&checkVal, sizeof(checkVal)) != sizeof(checkVal) || checkVal != size)
			//throw std::runtime_error("block format check failed");
			data.clear();
	}

	return data;
}

bool IConnection::writeBlock(const Bytes& data)
{
	uint32_t size = data.size();
	if (this->write(&size, sizeof(size)) != sizeof(size))
		return false;

	if (size > 0 && this->write(&data[0], data.size()) != size)
		return false;

	if (this->write(&size, sizeof(size)) != sizeof(size))
		return false;

	return true;
}

SerilizedObjs IConnection::_call(const SerilizedObjs& send)
{
	if (this->writeBlock(send.encode()))
	{
		Bytes ret = this->readBlock();
		return SerilizedObjs(ret);
	}
	return SerilizedObjs();
}

#if USE_SOCKPP==0

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

class ClientConnection::MyImpl
{
public:
	struct sockaddr_in serverAddr; // server address
	int sockfd = -1;			   // own file descriper


	void connect(const std::string& ip, int port) // server ip and server port
	{
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			throw std::runtime_error("Socket creation failed");
		}
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(port);
		if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0)
		{
			throw std::runtime_error("Invalid address/Address not supported");
		}
		if (::connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			throw std::runtime_error("Connection failed");
		}
	}
	void close()
	{
		if (sockfd >= 0)
		{
			::close(sockfd);
			sockfd = -1;
		}
	}

	int read(void* buf, int size) // void*ָ����һ��ͨ��ָ�����ͣ��ܹ�ָ���κ����͵����ݡ���ˣ���Ҫ�����ݴ��ݸ�ʹ��void*ָ����Ϊ�����ĺ���ʱ���轫���ݵĵ�ַת��Ϊvoid*
	{
		int bytesRead = recv(sockfd, buf, size, 0);
		if (bytesRead < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				throw std::runtime_error("Read timeout");
			}
			else
			{
				throw std::runtime_error("Read failed: " + std::string(strerror(errno)));
			}
		}
		return bytesRead;
	}

	int write(const void* buf, int size)
	{
		int bytesWritten = send(sockfd, buf, size, 0);
		if (bytesWritten < 0)
		{
			throw std::runtime_error("Write failed");
		}
		return bytesWritten;
	}
};


class ServerConnection::MyImpl
{
public:

	int sockfd;           // �������׽����ļ�������
	int clientSockfd;     // �ͻ����׽����ļ�������
	struct sockaddr_in serverAddr; // ��������ַ�ṹ

	MyImpl(int port) : sockfd(-1), clientSockfd(-1)
	{
		// �����������׽���
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
		}

		// ���÷�������ַ�Ͷ˿�
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(port);

		// ���׽��ֵ�ָ���ĵ�ַ�Ͷ˿�
		if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		{
			throw std::runtime_error("Bind failed: " + std::string(strerror(errno)));
		}

		// ���׽�������Ϊ����ģʽ����ʼ������������
		if (listen(sockfd, 5) < 0)
		{
			throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));
		}
	}
	~ServerConnection()
	{
		// �رշ������׽��ֺͿͻ����׽���
		if (sockfd >= 0)
		{
			::close(sockfd);
		}
		if (clientSockfd >= 0)
		{
			::close(clientSockfd);
		}
	}

	virtual int  read(void* buf, int  size)
	{
		// ���û�пͻ������ӣ�����һ���ͻ�������
		if (clientSockfd < 0)
		{
			struct sockaddr_in clientAddr;
			socklen_t clientAddrLen = sizeof(clientAddr);
			clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
			if (clientSockfd < 0)
			{
				std::cout << "no client connection! " << std::endl;
				return 0; //value of data in RPCServerConnection is 0
			}
		}

		// �ӿͻ��˶�ȡ����
		int bytesRead = recv(clientSockfd, buf, size, 0);
		if (bytesRead < 0)
		{
			throw std::runtime_error("Read failed: " + std::string(strerror(errno)));
		}
		return bytesRead;
	}

	virtual int  write(const void* buf, int size)
	{
		// ��ͻ��˷�������
		int bytesWritten = send(clientSockfd, buf, size, 0);
		if (bytesWritten < 0)
		{
			throw std::runtime_error("Write failed: " + std::string(strerror(errno)));
		}
		return bytesWritten;
	}

	virtual void close()
	{
		// �رտͻ����׽��ֺͷ������׽���
		if (clientSockfd >= 0)
		{
			::close(clientSockfd);
			clientSockfd = -1;
		}
		if (sockfd >= 0)
		{
			::close(sockfd);
			sockfd = -1;
		}
	}
};

#else

// #include "sockpp/tcp_connector.h"
// #include "sockpp/tcp_acceptor.h"
#include "sockpp/tcp_connector.h"
#include "sockpp/tcp_acceptor.h"


void init_sockpp()
{
	static bool _inited = false;
	if (!_inited)
	{
		sockpp::initialize();
		_inited = true;
	}
}

static auto NET_IO_TIMEOUT = std::chrono::microseconds(2000000);

class ClientConnection::MyImpl
{
	std::shared_ptr<sockpp::tcp_connector> _con;
public:
	MyImpl()
	{
		init_sockpp();
	}
	void connect(const std::string& ip, int port)
	{
		_con = std::shared_ptr<sockpp::tcp_connector>(new sockpp::tcp_connector({ ip,(in_port_t)port }));
		if (!_con)
			throw std::runtime_error(cv::format("failed to connect to server %s:%d", ip.c_str(), port));

		_con->read_timeout(NET_IO_TIMEOUT);
		_con->write_timeout(NET_IO_TIMEOUT);
	}

	void close()
	{
		if (_con)
		{
			if (!_con->close())
				throw std::runtime_error("failed to close socket client connection");
		}
	}

	int read(void* buf, int size)
	{
		return (int)_con->read_n(buf, size_t(size));
	}

	int write(const void* buf, int size)
	{
		return (int)_con->write_n(buf, size_t(size));
	}
};

class ServerConnection::MyImpl
{
	sockpp::tcp_socket _sock;
public:
	MyImpl()
	{
	}

	void setConnection(sockpp::tcp_socket sock)
	{
		_sock = std::move(sock);

		_sock.read_timeout(NET_IO_TIMEOUT);
		_sock.write_timeout(NET_IO_TIMEOUT);
	}

	void close()
	{
		if (_sock)
		{
			if (!_sock.close())
				throw std::runtime_error("failed to close socket connection");
		}
	}

	int read(void* buf, int size)
	{
		return (int)_sock.read_n(buf, (size_t)size);
	}

	int write(const void* buf, int size)
	{
		return (int)_sock.write_n(buf, (size_t)size);
	}
};

void runRPCServer(int port)
{
	sockpp::initialize();

	sockpp::tcp_acceptor acc((in_port_t)port);

	if (!acc)
	{
		throw std::runtime_error("Error creating the acceptor: " + acc.last_error_str());
	}
	cout << "Awaiting connections on port " << port << "..." << endl;

	ARServerManager& manager = ARServerManager::instance();

	while (true) 
	{
		sockpp::inet_address peer;

		// Accept a new client connection
		sockpp::tcp_socket sock = acc.accept(&peer);
		cout << "Received a connection request from " << peer << endl;

		

		if (!sock) {
			cerr << "Error accepting incoming connection: "
				<< acc.last_error_str() << endl;
		}
		else {
			// Create a thread and transfer the new stream to it.

			RPCServerConnectionPtr connection = RPCServerConnection::create();
			connection->name = peer.to_string();

			connection->ServerConnection::_impl->setConnection(std::move(sock));

			manager.setNewConnection(connection);
		}
	}
}

#endif



ClientConnection::ClientConnection()
{
	_impl = std::make_shared<MyImpl>();
}

void ClientConnection::connect(const std::string& ip, int port)
{
	_impl->connect(ip, port);
}

void ClientConnection::close()
{
	_impl->close();
}

int ClientConnection::read(void* buf, int size)
{
	return _impl->read(buf, size);
}

int ClientConnection::write(const void* buf, int size)
{
	return _impl->write(buf, size);
}



ServerConnection::ServerConnection()
{
	_impl = std::make_shared<MyImpl>();
}


void ServerConnection::close()
{
	_impl->close();
}

int ServerConnection::read(void* buf, int size)
{
	return _impl->read(buf, size);
}

int ServerConnection::write(const void* buf, int size)
{
	return _impl->write(buf, size);
}



