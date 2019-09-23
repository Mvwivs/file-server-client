
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

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

	std::tuple<std::size_t, std::string> getFileNameMessage();
	std::size_t getSessionMessage();
	std::string getFileListMessage();
	std::tuple<std::size_t, std::size_t> getFileMessage();

	std::vector<char> createMessage() const;
	Command getType() const;
	const char* getData() const;
	std::size_t getSize() const;
	void appendData(const std::string& fileName); // TODO: rename
	void appendData(std::size_t connCount);       // TODO: rename
	void setData(const std::vector<char> newData);

private:
	std::size_t getSizet(std::size_t pos);

};

#endif /* MESSAGE_HPP */
