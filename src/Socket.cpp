
#include "Socket.hpp"

Socket::Socket() : sock(-1) {
}

Socket::Socket(const std::string& address, int port) {
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		throw std::runtime_error("Unable to open socket");
	}
	int binAddr = inet_addr(address.c_str());
	if (binAddr == -1) {
		throw std::runtime_error("Unable to read ip addrees: wrong format or host names are "
					 "not supported");
	}
	server.sin_addr.s_addr = binAddr;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
		throw std::runtime_error("Unable to connect to server");
	}
}

Socket::Socket(int newSocket, struct sockaddr_in newServer) : sock(newSocket), server(newServer) {
}

Socket Socket::newConnection() const {
	int createdSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(createdSocket, (struct sockaddr*)&server, sizeof(server)) < 0) {
		throw std::runtime_error("Unable to connect to server");
	}
	Socket newSocket(*this);
	newSocket.sock = createdSocket;

	return newSocket;
}

int Socket::getSock() const {
	return sock;
}

Socket::~Socket() {
}

void Socket::closeSocket() {
	if (sock != -1) {
		close(sock);
	}
}

ServerSocket::ServerSocket() {
}

ServerSocket::~ServerSocket() {
}

ServerSocket::ServerSocket(int port) {
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0 || sock > 65536) {
		throw std::runtime_error("Unable to create socket");
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);
	if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
		throw std::runtime_error("Unable to bind port");
	}
}