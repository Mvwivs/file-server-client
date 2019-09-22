
#include "Message.hpp"

Message::Message() :
	header(Command::ERROR) {
}
Message::Message(Command c) :
	header(c) {
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

void Message::appendData(const std::string& fileName) {
	data.insert(data.end(), fileName.cbegin(), fileName.cend());
}

void Message::appendData(std::size_t connCount) {
	data.insert(data.end(), (const char*)&connCount, (const char*)(&connCount + 1));
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