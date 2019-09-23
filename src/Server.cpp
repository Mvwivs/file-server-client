
#include "Server.hpp"

Server::Server() : masterSock(-1), sessionCounter(1) {
}

Server::Server(int port) : sessionCounter(1) {
	masterSock = socket(AF_INET, SOCK_STREAM, 0);
	if (masterSock < 0 || masterSock > 65536) {
		throw std::runtime_error("Unable to create socket");
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(masterSock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) <
	    0) {
		throw std::runtime_error("Unable to bind port");
	}
}

Server::~Server() {
	clearEndedSessions();
	while (connections.size() != 0) {
		auto it = connections.begin();
		if (it->second.thread.joinable()) {
			it->second.thread.join();
		}
		int commandSocket = it->second.commandSocket;
		connections.erase(it);
		close(commandSocket);
	}
	if (masterSock != -1) {
		close(masterSock);
	}
}

void Server::startListening() {
	if (listen(masterSock, QUEUE_LENGTH) < 0) {
		throw std::runtime_error("Unable to start listenning");
	}

	while (true) {
		struct sockaddr_in cli_addr;
		socklen_t clilen = sizeof(cli_addr);
		int newSock =
		    accept(masterSock, (struct sockaddr*)&cli_addr, &clilen);

		std::size_t session = createSession(newSock);

#ifdef _DEBUG
		std::cout << "Client connected " << session << std::endl;
#endif
		{
			std::lock_guard<std::mutex> lck(mtx);
			if (connections.find(session) != connections.cend()) {
				connections[session].sockets.push_back(newSock);
				condVariable.notify_all();
			}
			else {
				connections[session] = UserProcesser{
				    std::thread(&Server::serveClient, this,
						newSock, session),
				    newSock, std::vector<int>()};
			}
		}
		clearEndedSessions();
	}
}

void Server::serveClient(int clientSocket, std::size_t client) {
	while (true) {
		try {
			Message msg = recieveMessage(clientSocket);
			if (msg.getType() == Message::Command::GET_FILE) {
				std::size_t connCount =
				    *(std::size_t*)&msg.getData()[0];
				std::string fileName(
				    msg.getData() + sizeof(std::size_t),
				    msg.getSize() - sizeof(std::size_t));
				if (isFileExists(fileName)) {
					if (connCount > FILE_CONNECTIONS) {
						connCount = FILE_CONNECTIONS;
					}
					Message command(
					    Message::Command::OPEN_CONN);
					command.appendData(connCount);
					std::size_t fileSize =
					    getFileSize(fileName);
					command.appendData(fileSize);
					sendMessage(command, clientSocket);
					sendFile(fileName, connCount, fileSize,
						 client);
				}
				else {
					Message command(
					    Message::Command::NOT_EXISTS);
					sendMessage(command, clientSocket);
				}
			}
			else if (msg.getType() ==
				 Message::Command::GET_FILE_LIST) {
				std::string fileList = getFileList();
				Message command(Message::Command::FILE_LIST);
				command.appendData(fileList);
				sendMessage(command, clientSocket);
			}
			else {
				throw std::runtime_error(
				    "Server got unexpected command in a "
				    "Message");
			}
		} catch (const HostDisconnectedException& e) {
#ifdef _DEBUG
			std::cout << "Client disconnected " << client
				  << std::endl;
#endif
			std::lock_guard<std::mutex> lck(mtx);
			endedSessions.push_back(client);
			return;
		}
	}
}

void Server::sendFile(const std::string& filename, std::size_t connCount,
		      std::size_t filesize, std::size_t client) {
	{
		std::unique_lock<std::mutex> lck(mtx);
		while (connections[client].sockets.size() != connCount)
			condVariable.wait(lck);
	}
	FileReader fr(filename, connCount, filesize);
	auto sendPart = [packet = BUFFER_SIZE, &fr](std::size_t id, int sock, std::size_t len) {
		std::size_t toRead = len;
		char buf[packet];
		while (true) {
			if (toRead < packet) {
				fr.read(id, buf, toRead);
				sendAllData(sock, toRead, buf);
				return;
			}
			fr.read(id, buf, packet);
			sendAllData(sock, packet, buf);

			toRead -= packet;
		}
	};

	std::vector<std::pair<std::thread, int>> pool;
	for (std::size_t i = 0; i < connCount; ++i) {
		std::size_t dataSize = filesize / connCount;
		int currSock = -1;
		if (i == connCount - 1) {
			dataSize += filesize % connCount;
		}
		{
			std::lock_guard<std::mutex> lck(mtx);
			currSock = connections[client].sockets[i];
		}
		pool.push_back(make_pair(
		    std::thread(sendPart, i, currSock, dataSize), currSock));
	}
	for (std::size_t i = 0; i < connCount; ++i) {
		pool[i].first.join();
		close(pool[i].second);
	}
	{
		std::lock_guard<std::mutex> lck(mtx);
		connections[client].sockets.clear();
	}
}

Message Server::recieveMessage(int sock) const {
	Message msg(*(Message::Command*)(&readAllData(sizeof(Message::Command),
						      sock)[0]));
	if (msg.getType() != Message::Command::ERROR) {
		std::size_t dataSize =
		    *(std::size_t*)&readAllData(sizeof(std::size_t), sock)[0];
		msg.setData(readAllData(dataSize, sock));
	}
	return msg;
}

std::vector<char> Server::readAllData(std::size_t len, int sock) const {
	std::size_t toRead = len;
	std::vector<char> data;
	char buf[BUFFER_SIZE];
	char* ptr = buf;

	while (toRead > 0) {
		ssize_t recieved =
		    read(sock, ptr, std::min(toRead, BUFFER_SIZE));
		if (recieved == -1) {
			throw std::runtime_error(
			    "Server got an error while recieving data");
		}
		if (recieved == 0) {
			throw HostDisconnectedException();
		}
		data.insert(data.end(), ptr, ptr + recieved);
		toRead -= recieved;
	}

	return data;
}

std::vector<char> Server::sendAllData(int sock, std::size_t len,
				      const char* buf) {
	std::size_t toSend = len;
	std::vector<char> data;

	while (toSend > 0) {
		ssize_t sent = write(sock, buf, toSend);
		if (sent == -1) {
			throw std::runtime_error(
			    "Error while sending data by server");
		}
		if (sent == 0) {
			throw std::runtime_error("Server cannot send data");
		}
		buf += sent;
		toSend -= sent;
	}

	return data;
}

void Server::sendMessage(const Message& msg, int clientSocket) const {
	std::vector<char> data = msg.createMessage();
	sendAllData(clientSocket, data.size(), &data[0]);
}

std::size_t Server::createSession(int sock) {
	Message recieved = recieveMessage(sock);
	if (recieved.getType() != Message::Command::SESSION_ID) {
		throw std::runtime_error("Server unable to create session");
	}
	std::size_t session = *(std::size_t*)&recieved.getData()[0];
	if (session == 0) {
		session = sessionCounter;
		++sessionCounter;
	}

	Message msg(Message::Command::SESSION_ID);
	msg.appendData(session);
	sendMessage(msg, sock);

	return session;
}

void Server::clearEndedSessions() {
	std::lock_guard<std::mutex> lck(mtx);
	while (endedSessions.size() != 0) {
		std::size_t last = endedSessions.back();
		connections[last].thread.join();
		close(connections[last].commandSocket);

		connections.erase(last);
		endedSessions.pop_back();
	}
}