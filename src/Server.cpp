
#include "Server.hpp"

Server::Server() : masterSock(), sessionCounter(1) {
}

Server::Server(int port) : masterSock(port), sessionCounter(1) {
}

Server::~Server() {
	clearEndedSessions();
	while (connections.size() != 0) {
		auto it = connections.begin();
		if (it->second.thread.joinable()) {
			it->second.thread.join();
		}
		Socket commandSocket = it->second.commandSocket;
		connections.erase(it);
		commandSocket.closeSocket();
	}
	masterSock.closeSocket();
}

void Server::startListening() {
	masterSock.listenSocket(QUEUE_LENGTH);

	while (true) {
		Socket newSock = masterSock.acceptSocket();
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
				connections[session] =
				    UserProcesser{std::thread(&Server::serveClient, this, newSock, session),
						  newSock, std::vector<Socket>()};
			}
		}
		clearEndedSessions();
	}
}

void Server::serveClient(const Socket& clientSocket, std::size_t client) {
	while (true) {
		try {
			Message msg = Message::recieveMessage(clientSocket);
			switch (msg.getType()) {
				case Message::Command::GET_FILE: {
					auto [connCount, fileName] = msg.getFileNameMessage();
					if (isFileExists(fileName)) {
						connCount = std::min(connCount, FILE_CONNECTIONS);
						std::size_t fileSize = getFileSize(fileName);
						Message command(Message::Command::OPEN_CONN, connCount, fileSize);
						Message::sendMessage(command, clientSocket);
						sendFile(fileName, connCount, fileSize, client);
					}
					else {
						Message command(Message::Command::NOT_EXISTS);
						Message::sendMessage(command, clientSocket);
					}
					break;
				}
				case Message::Command::GET_FILE_LIST: {
					Message command(Message::Command::FILE_LIST, getFileList());
					Message::sendMessage(command, clientSocket);
					break;
				}
				default: {
					throw std::runtime_error("Server got unexpected command in a Message");
					break;
				}
			}
		} catch (const HostDisconnectedException& e) {
#ifdef _DEBUG
			std::cout << "Client disconnected " << client << std::endl;
#endif
			std::lock_guard<std::mutex> lck(mtx);
			endedSessions.push_back(client);
			return;
		}
	}
}

void Server::sendFile(const std::string& filename, std::size_t connCount, std::size_t filesize,
		      std::size_t client) {
	{
		std::unique_lock<std::mutex> lck(mtx);
		while (connections[client].sockets.size() != connCount)
			condVariable.wait(lck);
	}
	FileReader fr(filename, connCount, filesize);

	auto sendPart = [packet = BUFFER_SIZE, &fr](std::size_t id, const Socket& sock, std::size_t len) {
		std::size_t toRead = len;
		char buf[packet];
		while (true) {
			if (toRead < packet) {
				fr.read(id, buf, toRead);
				sock.sendAllData(buf, toRead);
				return;
			}
			fr.read(id, buf, packet);
			sock.sendAllData(buf, packet);

			toRead -= packet;
		}
	};

	std::vector<std::pair<std::thread, Socket>> pool;
	for (std::size_t i = 0; i < connCount; ++i) {
		std::size_t dataSize = filesize / connCount;
		Socket currSock;
		if (i == connCount - 1) {
			dataSize += filesize % connCount;
		}
		{
			std::lock_guard<std::mutex> lck(mtx);
			currSock = connections[client].sockets[i];
		}
		pool.push_back(make_pair(std::thread(sendPart, i, currSock, dataSize), currSock));
	}
	for (std::size_t i = 0; i < connCount; ++i) {
		pool[i].first.join();
		pool[i].second.closeSocket();
	}
	{
		std::lock_guard<std::mutex> lck(mtx);
		connections[client].sockets.clear();
	}
}

std::size_t Server::createSession(const Socket& sock) {
	Message recieved = Message::recieveMessage(sock);
	if (recieved.getType() != Message::Command::SESSION_ID) {
		throw std::runtime_error("Server unable to create session");
	}
	std::size_t session = recieved.getSessionMessage();
	if (session == 0) {
		session = sessionCounter;
		++sessionCounter;
	}

	Message msg(Message::Command::SESSION_ID, session);
	Message::sendMessage(msg, sock);

	return session;
}

void Server::clearEndedSessions() {
	std::lock_guard<std::mutex> lck(mtx);
	while (endedSessions.size() != 0) {
		std::size_t last = endedSessions.back();
		connections[last].thread.join();
		connections[last].commandSocket.closeSocket();

		connections.erase(last);
		endedSessions.pop_back();
	}
}