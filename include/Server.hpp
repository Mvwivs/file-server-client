
#ifndef SERVER_HPP
#define SERVER_HPP

#include "FileAccess.hpp"
#include "Message.hpp"
#include "Socket.hpp"

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <vector>

struct UserProcesser {
	std::thread thread;
	Socket commandSocket;
	std::vector<Socket> sockets;
};

class Server {
private:
	std::mutex mtx;
	std::condition_variable condVariable;
	ServerSocket masterSock;
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
	void serveClient(const Socket& clienSocket, std::size_t client);
	Message recieveMessage(const Socket& sock) const;
	void sendMessage(const Message& msg, const Socket& clientSocket) const;
	void sendFile(const std::string& filename, std::size_t connCount, std::size_t filesize,
		      std::size_t client);
	std::size_t createSession(const Socket& sock);
	void clearEndedSessions();
};

#endif /* SERVER_HPP */
