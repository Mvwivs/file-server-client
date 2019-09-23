
#include "Client.hpp"

Client::Client() : mainSocket(), session(0) {
}

Client::~Client() {
	mainSocket.closeSocket();
}

Client::Client(const std::string& address, int port) : mainSocket(address, port), session(0) {
	createSession(mainSocket);
}

std::vector<std::string> Client::getFileList() const {
	Message msg(Message::Command::GET_FILE_LIST);
	sendMessage(msg, mainSocket);

	Message recieved = recieveMessage(mainSocket);
	if (recieved.getType() != Message::Command::FILE_LIST) {
		throw std::runtime_error("Recieved unexpected data type");
	}
	std::vector<std::string> fileList;
	std::stringstream ss(recieved.getFileListMessage());
	std::string item;
	while (std::getline(ss, item, '\n')) {
		fileList.push_back(std::move(item));
	}

	return fileList;
}

void Client::getFile(std::string fileName) {
	Message msg(Message::Command::GET_FILE, PREFFERED_CONNECTIONS, fileName);

	sendMessage(msg, mainSocket);

	Message recieved = recieveMessage(mainSocket);
	if (recieved.getType() == Message::Command::NOT_EXISTS) {
		throw std::runtime_error("No such file");
	}
	auto [connCount, fileSize] = recieved.getFileMessage();
	while (isFileExists(fileName)) {
		fileName = std::string("_").append(fileName);
	}
	readFile(connCount, fileName, fileSize);
}

void Client::sendMessage(const Message& msg, const Socket& sock) const {
	std::vector<char> data = msg.createMessage();
	write(sock.getSock(), &data[0], data.size());
}

Message Client::recieveMessage(const Socket& sock) const {
	Message msg(*(Message::Command*)(&readAllData(sizeof(Message::Command), sock)[0]));
	if (msg.getType() == Message::Command::ERROR) {
		throw std::runtime_error("Server returned error");
	}
	std::size_t dataSize = *(std::size_t*)&readAllData(sizeof(std::size_t), sock)[0];
	msg.setData(readAllData(dataSize, sock));

	return msg;
}

std::vector<char> Client::readAllData(std::size_t len, const Socket& sock) const {
	std::size_t toRead = len;
	std::vector<char> data;
	char buf[READ_BUFFER];
	char* ptr = buf;

	while (toRead > 0) {
		ssize_t recieved = read(sock.getSock(), ptr, std::min(toRead, READ_BUFFER));
		if (recieved == -1) {
			throw std::runtime_error("Error while recieving data");
		}
		if (recieved == 0) {
			throw std::runtime_error("Cannot recieve expected data");
		}
		data.insert(data.end(), ptr, ptr + recieved);
		toRead -= recieved;
	}

	return data;
}

void Client::readFile(std::size_t connCount, const std::string& filename, std::size_t fileSize) {

	FileWriter fw(filename, connCount, fileSize);

	auto readPart = [packet = READ_BUFFER, &fw](std::size_t id, const Socket& sock, std::size_t len) {
		std::size_t toRead = len;
		char buf[packet];

		while (toRead > 0) {
			ssize_t recieved = read(sock.getSock(), buf, packet);
			if (recieved == -1) {
				throw std::runtime_error("Error while recieving data");
			}
			if (recieved == 0) {
				throw std::runtime_error("Cannot recieve expected data");
			}
			std::vector<char> data(buf, buf + recieved);
			fw.write(id, data);

			toRead -= recieved;
		}
	};

	std::vector<std::pair<std::thread, Socket>> pool;
	for (std::size_t i = 0; i < connCount; ++i) {
		Socket currSock(mainSocket.newConnection());
		createSession(currSock);

		std::size_t dataSize = fileSize / connCount;
		if (i == connCount - 1) {
			dataSize += fileSize % connCount;
		}
		pool.push_back(make_pair(std::thread(readPart, i, currSock, dataSize), currSock));
	}

	for (std::size_t i = 0; i < connCount; ++i) {
		pool[i].first.join();
		pool[i].second.closeSocket();
	}
}

void Client::createSession(const Socket& sock) {
	Message msg(Message::Command::SESSION_ID, session);
	sendMessage(msg, sock);

	Message recieved = recieveMessage(sock);
	if (recieved.getType() != Message::Command::SESSION_ID) {
		throw std::runtime_error("Unable to create session");
	}
	session = recieved.getSessionMessage();
}
