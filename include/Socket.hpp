
#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <arpa/inet.h>
#include <array>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class Socket {
protected:
	int sock;
	struct sockaddr_in server;

	static const std::size_t READ_BUFFER_SIZE;

public:
	Socket();
	virtual ~Socket();

	Socket(const std::string& address, int port);
	Socket(int newSocket, struct sockaddr_in newServer);

	Socket newConnection() const;
	int getSock() const;
	void closeSocket();
	void sendAllData(const char* buf, std::size_t len) const;
	std::vector<char> readAllData(std::size_t len) const;
};

class ServerSocket : public Socket {
private:
public:
	ServerSocket();
	ServerSocket(int port);
	~ServerSocket();

	void listenSocket(std::size_t queueLength) const;
	Socket acceptSocket() const;
};

class HostDisconnectedException : public std::exception {
	std::string what_msg;

public:
	HostDisconnectedException() : what_msg("Host disconnected") {
	}
	const char* what() {
		return what_msg.c_str();
	}
};

#endif /* SOCKET_HPP */
