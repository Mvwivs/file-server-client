
#include "Client.hpp"
#include <string>
#include <memory>
#include <iostream>
#include <vector>

class ConsoleClient {
private:
	std::unique_ptr<Client> client;

public:
	ConsoleClient();
	~ConsoleClient();

	void readCommands();

private:
	void connect(const std::string& address, int port);
	void disconnect();
	void getFileList();
	void getFile(const std::string& name);
};
