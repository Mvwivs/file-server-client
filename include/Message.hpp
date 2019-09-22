
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <vector>
#include <string>

class Message
{
public:
	enum class Command {
		ERROR = 0, SESSION_ID, GET_FILE_LIST, GET_FILE, FILE_LIST, OPEN_CONN
	};
private:
	Command header;
	std::vector<char> data;

public:
	Message();
	~Message();

	Message(Command c);

	std::vector<char> createMessage() const;
	Command getType() const;
	const char* getData() const;
	std::size_t getSize() const;
	void appendData(const std::string& fileName);	// TODO: rename
	void appendData(std::size_t connCount);		// TODO: rename
	void setData(const std::vector<char> newData);
};

#endif /* MESSAGE_HPP */
