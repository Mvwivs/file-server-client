
#include <arpa/inet.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class Client {
private:
	int sock;

public:
	~Client();

	Client(const std::string& address, int port);
	std::vector<std::string> getFileList();
	void getFile(const std::string& fileName);

private:
	Client();
};