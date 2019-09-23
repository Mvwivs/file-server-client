
#include "Message.hpp"

Message::Message() : header(Command::ERROR) {
}
Message::Message(Command c) : header(c) {
}

Message::~Message() {
}

std::vector<char> Message::createMessage() const {
	std::vector<char> msg;
	std::size_t dataSize = data.size();
	msg.reserve(sizeof(Command) + sizeof(std::size_t) + data.size());
	msg.insert(msg.end(), (const char*)&header, (const char*)(&header + 1));
	msg.insert(msg.end(), (const char*)&dataSize, (const char*)(&dataSize + 1));
	msg.insert(msg.end(), std::begin(data), std::end(data));

	return msg;
}

std::tuple<std::size_t, std::string> Message::getFileNameMessage() const {
	std::size_t connCount = getSizet(0);
	const char* stringStart = data.data() + sizeof(std::size_t);
	std::size_t stringLength = getSize() - sizeof(std::size_t);
	std::string fileName(stringStart, stringLength);

	return std::make_tuple(connCount, fileName);
}

std::tuple<std::size_t, std::size_t> Message::getFileMessage() const {
	std::size_t connCount = getSizet(0);
	std::size_t fileSize = getSizet(sizeof(std::size_t));

	return std::make_tuple(connCount, fileSize);
}

std::string Message::getFileListMessage() const {
	return std::string(getData(), getSize());
}

std::size_t Message::getSessionMessage() const {
	return getSizet(0);
}

std::size_t Message::getSizet(std::size_t pos) const {
	return *(std::size_t*)&getData()[pos];
}

Message::Message(Command c, const std::string& fileList) : header(c) {
	appendData(fileList);
}

Message::Message(Command c, std::size_t session) : header(c) {
	appendData(session);
}

Message::Message(Command c, std::size_t connCount, const std::string& fileName) : header(c) {
	appendData(connCount);
	appendData(fileName);
}

Message::Message(Command c, std::size_t connCount, std::size_t fileSize) : header(c) {
	appendData(connCount);
	appendData(fileSize);
}

void Message::appendData(const std::string& str) {
	data.insert(data.end(), str.cbegin(), str.cend());
}

void Message::appendData(std::size_t num) {
	data.insert(data.end(), (const char*)&num, (const char*)(&num + 1));
}

void Message::setData(const std::vector<char> newData) {
	data.insert(data.begin(), std::begin(newData), std::end(newData));
}

Message::Command Message::getType() const {
	return header;
}

const char* Message::getData() const {
	return data.data();
}

std::size_t Message::getSize() const {
	return data.size();
}

void Message::sendMessage(const Message& msg, const Socket& clientSocket) {
	std::vector<char> data = msg.createMessage();
	clientSocket.sendAllData(&data[0], data.size());
}

Message Message::recieveMessage(const Socket& sock) {
	Message msg(*(Message::Command*)(&sock.readAllData(sizeof(Message::Command))[0]));
	if (msg.getType() != Message::Command::ERROR) {
		std::size_t dataSize = *(std::size_t*)&sock.readAllData(sizeof(std::size_t))[0];
		msg.setData(sock.readAllData(dataSize));
	}
	return msg;
}
