
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "FileAccess.hpp"
#include "Message.hpp"
#include "Socket.hpp"

#include <arpa/inet.h>
#include <future>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class Client {
private:
	Socket mainSocket;
	std::size_t session;
	struct sockaddr_in server;

	const std::size_t PREFFERED_CONNECTIONS = 1;
	const std::size_t READ_BUFFER = 1024 * 1024;

public:
	~Client();

	Client(const std::string& address, int port);
	std::vector<std::string> getFileList() const;
	void getFile(std::string fileName);

private:
	Client();
	void createSession(const Socket& sock);
	void readFile(std::size_t connCount, const std::string& filename, std::size_t fileSize);
	void sendMessage(const Message& msg, const Socket& sock) const;
	std::vector<char> readAllData(std::size_t len, const Socket& sock) const;
	Message recieveMessage(const Socket& sock) const;
	void createConnection(const std::string& address, int port);
	int createConnection();
};

#endif /* CLIENT_HPP */
