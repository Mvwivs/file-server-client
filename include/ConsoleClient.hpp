
#include "Client.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
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
	bool isConnected();

	static const char* commands;
};
