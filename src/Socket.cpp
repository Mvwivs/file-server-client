
#include "Socket.hpp"

const std::size_t Socket::READ_BUFFER_SIZE = 1024;

Socket::Socket() : sock(-1) {
}

Socket::~Socket() {
}

void Socket::closeSocket() {
	if (sock != -1) {
		close(sock);
	}
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

void Socket::sendAllData(const char* buf, std::size_t len) const {
	std::size_t toSend = len;

	while (toSend > 0) {
		ssize_t sent = write(sock, buf, toSend);
		if (sent == -1) {
			throw std::runtime_error("Error while sending data");
		}
		if (sent == 0) {
			throw std::runtime_error("Unable to send data");
		}
		buf += sent;
		toSend -= sent;
	}
}

std::vector<char> Socket::readAllData(std::size_t len) const {
	std::size_t toRead = len;
	std::vector<char> data;
	data.reserve(len);
	std::array<char, READ_BUFFER_SIZE> buf;

	while (toRead > 0) {
		ssize_t recieved = read(sock, buf.data(), std::min(toRead, READ_BUFFER_SIZE));
		if (recieved == -1) {
			throw std::runtime_error("Error while recieving data");
		}
		if (recieved == 0) {
			throw HostDisconnectedException();
		}
		data.insert(data.end(), buf.cbegin(), buf.cbegin() + recieved);
		toRead -= recieved;
	}

	return data;
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

void ServerSocket::listenSocket(std::size_t queueLength) const {
	if (listen(sock, queueLength) < 0) {
		throw std::runtime_error("Unable to start listenning");
	}
}

Socket ServerSocket::acceptSocket() const {
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	int acceptedSocket = accept(sock, (struct sockaddr*)&cli_addr, &clilen);
	Socket newSock(acceptedSocket, cli_addr);

	return newSock;
}
