
#include "Client.hpp"

Client::Client() : mainSocket(-1), session(0) {
}

Client::~Client() {
	if (mainSocket != -1) {
		close(mainSocket);
	}
}

Client::Client(const std::string& address, int port) : session(0) {
	createConnection(address, port);
}

void Client::createConnection(const std::string& address, int port) {
	mainSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mainSocket == -1) {
		throw std::runtime_error("Unable to open socket");
	}
	int binAddr = inet_addr(address.c_str());
	if (binAddr == -1) {
		throw std::runtime_error(
		    "Unable to read ip addrees: wrong format or host names are "
		    "not supported");
	}
	server.sin_addr.s_addr = binAddr;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (connect(mainSocket, (struct sockaddr*)&server, sizeof(server)) <
	    0) {
		throw std::runtime_error("Unable to connect to server");
	}

	createSession(mainSocket);
}

int Client::createConnection() {
	int newSock = socket(AF_INET, SOCK_STREAM, 0);
	if (newSock == -1) {
		throw std::runtime_error("Unable to open socket");
	}
	if (connect(newSock, (struct sockaddr*)&server, sizeof(server)) < 0) {
		throw std::runtime_error("Unable to connect to server");
	}

	createSession(newSock);

	return newSock;
}

std::vector<std::string> Client::getFileList() const {
	Message msg(Message::Command::GET_FILE_LIST);
	sendMessage(msg, mainSocket);

	Message recieved = recieveMessage(mainSocket);
	if (recieved.getType() != Message::Command::FILE_LIST) {
		throw std::runtime_error("Recieved unexpected data type");
	}
	std::vector<std::string> fileList;
	std::stringstream ss(
	    std::string(recieved.getData(), recieved.getSize()));
	std::string item;
	while (std::getline(ss, item, '\n')) {
		fileList.push_back(std::move(item));
	}

	return fileList;
}

void Client::getFile(const std::string& fileName) {
	Message msg(Message::Command::GET_FILE);
	msg.appendData(PREFFERED_CONNECTIONS);
	msg.appendData(fileName);
	sendMessage(msg, mainSocket);

	Message recieved = recieveMessage(mainSocket);
	if (recieved.getType() == Message::Command::NOT_EXISTS) {
		throw std::runtime_error("No such file");
	}
	std::size_t connCount = *(std::size_t*)&recieved.getData()[0];
	std::size_t fileSize =
	    *(std::size_t*)&recieved.getData()[sizeof(std::size_t)];
	readFile(connCount, fileName, fileSize);
}

void Client::sendMessage(const Message& msg, int sock) const {
	std::vector<char> data = msg.createMessage();
	write(sock, &data[0], data.size());
}

Message Client::recieveMessage(int sock) const {
	Message msg(*(Message::Command*)(&readAllData(sizeof(Message::Command),
						      sock)[0]));
	if (msg.getType() == Message::Command::ERROR) {
		throw std::runtime_error("Server returned error");
	}
	std::size_t dataSize =
	    *(std::size_t*)&readAllData(sizeof(std::size_t), sock)[0];
	msg.setData(readAllData(dataSize, sock));

	return msg;
}

std::vector<char> Client::readAllData(std::size_t len, int sock) const {
	std::size_t toRead = len;
	std::vector<char> data;
	char buf[READ_BUFFER];
	char* ptr = buf;

	while (toRead > 0) {
		ssize_t recieved =
		    read(sock, ptr, std::min(toRead, READ_BUFFER));
		if (recieved == -1) {
			throw std::runtime_error("Error while recieving data");
		}
		if (recieved == 0) {
			throw std::runtime_error(
			    "Cannot recieve expected data");
		}
		data.insert(data.end(), ptr, ptr + recieved);
		toRead -= recieved;
	}

	return data;
}

void Client::readFile(std::size_t connCount, const std::string& filename,
		      std::size_t fileSize) {

	FileWriter fw(filename, connCount, fileSize);

	auto readPart = [&fw](std::size_t id, int sock, std::size_t len) {
		std::size_t toRead = len;
		const std::size_t packet = 1024;
		char buf[packet];

		while (toRead > 0) {
			ssize_t recieved = read(sock, buf, packet);
			if (recieved == -1) {
				throw std::runtime_error(
				    "Error while recieving data");
			}
			if (recieved == 0) {
				throw std::runtime_error(
				    "Cannot recieve expected data");
			}
			std::vector<char> data(buf, buf + recieved);
			fw.write(id, data);

			toRead -= recieved;
		}
	};

	std::vector<std::pair<std::thread, int>> pool;
	for (std::size_t i = 0; i < connCount; ++i) {
		int currSock = createConnection();
		std::size_t dataSize = fileSize / connCount;
		if (i == connCount - 1) {
			dataSize += fileSize % connCount;
		}
		pool.push_back(make_pair(
		    std::thread(readPart, i, currSock, dataSize), currSock));
	}
	for (std::size_t i = 0; i < connCount; ++i) {
		pool[i].first.join();
		close(pool[i].second);
	}
}

void Client::createSession(int sock) {
	Message msg(Message::Command::SESSION_ID);
	msg.appendData(session);
	sendMessage(msg, sock);

	Message recieved = recieveMessage(sock);
	if (recieved.getType() != Message::Command::SESSION_ID) {
		throw std::runtime_error("Unable to create session");
	}
	session = *(std::size_t*)&recieved.getData()[0];
}
