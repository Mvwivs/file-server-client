
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "Socket.hpp"

#include <string>
#include <tuple>
#include <vector>

class Message {
public:
	enum class Command { ERROR = 0, SESSION_ID, GET_FILE_LIST, GET_FILE, FILE_LIST, OPEN_CONN, NOT_EXISTS };

private:
	Command header;
	std::vector<char> data;

public:
	Message();
	~Message();

	Message(Command c);
	Message(Command c, const std::string& fileList);
	Message(Command c, std::size_t session);
	Message(Command c, std::size_t connCount, const std::string& fileName);
	Message(Command c, std::size_t connCount, std::size_t fileSize);

	std::tuple<std::size_t, std::string> getFileNameMessage() const;
	std::size_t getSessionMessage() const;
	std::string getFileListMessage() const;
	std::tuple<std::size_t, std::size_t> getFileMessage() const;

	std::vector<char> createMessage() const;
	Command getType() const;
	const char* getData() const;
	std::size_t getSize() const;
	void setData(const std::vector<char> newData);

	static void sendMessage(const Message& msg, const Socket& clientSocket);
	static Message recieveMessage(const Socket& sock);
	
private:
	std::size_t getSizet(std::size_t pos) const;
	void appendData(const std::string& fileName);
	void appendData(std::size_t connCount);
};

#endif /* MESSAGE_HPP */
