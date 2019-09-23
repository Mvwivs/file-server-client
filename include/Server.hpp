
#ifndef SERVER_HPP
#define SERVER_HPP

#include "FileAccess.hpp"
#include "Message.hpp"

#include <arpa/inet.h>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

struct UserProcesser {
	std::thread thread;
	int commandSocket;
	std::vector<int> sockets;
};

class Server {
private:
	std::mutex mtx;
	std::condition_variable condVariable;
	struct sockaddr_in serv_addr;
	int masterSock;
	std::unordered_map<std::size_t, UserProcesser> connections;
	std::vector<std::size_t> endedSessions;
	std::size_t sessionCounter;

	const std::size_t QUEUE_LENGTH = 5;
	const std::size_t FILE_CONNECTIONS = 4;
	const std::size_t BUFFER_SIZE = 1024;

public:
	Server(int port);
	~Server();
	void startListening();

private:
	Server();
	void serveClient(int clienSocket, std::size_t client);
	Message recieveMessage(int sock) const;
	std::vector<char> readAllData(std::size_t len, int sock) const;
	void sendMessage(const Message& msg, int clientSocket) const;
	void sendFile(const std::string& filename, std::size_t connCount,
		      std::size_t filesize, std::size_t client);
	static std::vector<char> sendAllData(int sock, std::size_t len,
					     const char* buf);
	std::size_t createSession(int sock);
	void clearEndedSessions();
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

#endif /* SERVER_HPP */
