
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <arpa/inet.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <future>
#include <iostream>

#include "Message.hpp"
#include "FileWriter.hpp"

class Client {
private:
	int mainSocket;
	std::size_t session;
	struct sockaddr_in server;

	const std::size_t PREFFERED_CONNECTIONS = 2;
	const std::size_t READ_BUFFER = 1024;

public:
	~Client();

	Client(const std::string& address, int port);
	std::vector<std::string> getFileList() const;
	void getFile(const std::string& fileName);

private:
	Client();
	void sendMessage(const Message& msg, int sock) const;
	std::vector<char> readAllData(std::size_t len, int sock) const;
	Message recieveMessage(int sock) const;
	void readFile(std::size_t connCount, const std::string& filename, std::size_t fileSize);
	void createConnection(const std::string& address, int port);
	int createConnection();
	void createSession(int sock);
};

#endif /* CLIENT_HPP */
