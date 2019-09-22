
#include "Server.hpp"

Server::Server() : masterSock(-1), sessionCounter(1) {
}

Server::Server(int port) : sessionCounter(1) {
	masterSock = socket(AF_INET, SOCK_STREAM, 0);
	if (masterSock == -1) {
		throw std::runtime_error("Unable to create socket");
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(masterSock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) <
	    0) {
		throw std::runtime_error("Unable to bind port");
	}

	if (listen(masterSock, QUEUE_LENGTH) < 0) {
		throw std::runtime_error("Unable to start listenning");
	}
	startListening();
}

Server::~Server() {
	if (masterSock != -1) {
		close(masterSock);
	}
	while (endedSessions.size() != 0) {
		std::size_t last = *endedSessions.cend();
		connections[last].thread.join();
		connections.erase(last);
		endedSessions.pop_back();
	}
}

void Server::startListening() {
	while (true) {
		struct sockaddr_in cli_addr;
		socklen_t clilen = sizeof(cli_addr);
		int newSock =
		    accept(masterSock, (struct sockaddr*)&cli_addr, &clilen);

		std::size_t session = createSession(newSock);

		{
			std::lock_guard<std::mutex> lck(mtx);
			if (connections.find(session) != connections.cend()) {
				connections[session].sockets.push_back(newSock);
			}
			else {
				connections[session] = UserProcesser();
				connections[session].thread =
				    std::thread(&Server::serveClient, this,
						newSock, session);
			}

			while (endedSessions.size() != 0) {
				std::size_t last = *endedSessions.cend();
				connections[last].thread.join();
				connections.erase(last);
				endedSessions.pop_back();
			}
		}
	}
}

void Server::serveClient(int clientSocket, std::size_t client) {
	while (true) {
		try {
			Message msg = recieveMessage(clientSocket);
			if (msg.getType() == Message::Command::GET_FILE) {
				std::size_t connCount =
				    *(std::size_t*)&msg.getData()[0];
				std::string fileName(msg.getData() +
						     sizeof(std::size_t));
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
					    Message::Command::ERROR);
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
				    "Unexpected command in a Message");
			}
		} catch (const HostDisconnectedException& e) {
			std::lock_guard<std::mutex> lck(mtx);
			endedSessions.push_back(client);
			return;
		}
	}
}

void Server::sendFile(const std::string& filename, std::size_t connCount,
		      std::size_t filesize, std::size_t client) {
	while (connections[client].sockets.size() != connCount)
		sleep(1); //!!!!!!!!!!!!!!

	FileReader fr(filename, connCount, filesize);
	auto sendPart = [&fr](std::size_t id, int sock, std::size_t len) {
		std::size_t toRead = len;
		const std::size_t packet = 1024;
		char buf[packet];
		if (toRead < packet) {
			fr.read(id, buf, toRead);
			sendAllData(sock, toRead, buf);
			return;
		}
		fr.read(id, buf, packet);
		sendAllData(sock, packet, buf);

		toRead -= packet;
	};

	std::vector<std::pair<std::thread, int>> pool;
	for (std::size_t i = 0; i < connCount; ++i) {
		std::size_t dataSize = filesize / connCount;
		int currSock = -1;
		if ((i == connCount - 1) && (filesize % connCount != 0)) {
			dataSize += 1;
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
}

std::string Server::getFileList() const {
	namespace fs = std::experimental::filesystem;
	std::stringstream fileList;
	for (const auto& entry : fs::directory_iterator(".")) {
		if (entry.status().type() != fs::file_type::directory) {
			fileList << entry.path() << '\n';
		}
	}
	return fileList.str();
}

bool Server::isFileExists(const std::string& fileName) const {
	std::ifstream f(fileName);
	return f.good();
}

std::size_t Server::getFileSize(const std::string& filename) const {
	namespace fs = std::experimental::filesystem;
	return fs::file_size(filename);
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
		ssize_t recieved = read(sock, ptr, toRead);
		if (recieved == -1) {
			throw std::runtime_error("Error while recieving data");
		}
		if (recieved == 0) {
			throw std::runtime_error(
			    "Cannot recieve expected data");
		}
		data.insert(data.end(), ptr, ptr + recieved);
		ptr += recieved;
		toRead -= recieved;
	}

	return data;
}

std::vector<char> Server::sendAllData(int sock, std::size_t len,
				      const char* buf) {
	std::size_t toRead = len;
	std::vector<char> data;

	while (toRead > 0) {
		ssize_t sent = write(sock, buf, toRead);
		if (sent == -1) {
			throw std::runtime_error("Error while sending data");
		}
		if (sent == 0) {
			throw std::runtime_error("Cannot send data");
		}
		buf += sent;
		toRead -= sent;
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
		throw std::runtime_error("Unable to create session");
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