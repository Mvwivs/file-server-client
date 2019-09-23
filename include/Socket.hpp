
#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <arpa/inet.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class Socket {
protected:
	int sock;
	struct sockaddr_in server;

public:
	Socket();
	virtual ~Socket();

	Socket(const std::string& address, int port);
	Socket(int newSocket, struct sockaddr_in newServer);

	Socket newConnection() const;
	int getSock() const;
	void closeSocket();
};

typedef Socket ClientSocket;

class ServerSocket : public Socket {
private:
public:
	ServerSocket();
	ServerSocket(int port);
	~ServerSocket();
};

#endif /* SOCKET_HPP */
