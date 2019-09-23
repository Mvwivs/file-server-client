
#include "ConsoleClient.hpp"

ConsoleClient::ConsoleClient() : client(nullptr) {
}

ConsoleClient::~ConsoleClient() {
}

void ConsoleClient::readCommands() {
	std::string command;
	std::string parameter1, parameter2;

	while (true) {
		std::cout << "> ";
		std::cin >> command;
		if (command == "connect") {
			std::cin >> parameter1 >> parameter2;
			connect(parameter1, std::stoi(parameter2));
		}
		else if (command == "disconnect") {
			disconnect();
		}
		else if (command == "list") {
			getFileList();
		}
		else if (command == "get") {
			std::cin >> parameter1;
			getFile(parameter1);
		}
		else if (command == "exit") {
			return;
		}
		else if (command == "help") {
			std::cout << commands;
		}
		else {
			std::cerr << "Unknown command" << std::endl;
		}
	}
}

void ConsoleClient::connect(const std::string& address, int port) {
	try {
		client.reset(new Client(address, port));
		std::cout << "Connected" << std::endl;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

void ConsoleClient::disconnect() {
	client.reset(nullptr);
	std::cout << "Disconnected" << std::endl;
}

void ConsoleClient::getFileList() {
	try {
		if (!isConnected()) {
			std::cout << "Not connected" << std::endl;
			return;
		}
		std::vector<std::string> files(
		    std::move(client->getFileList()));
		for (const auto& f : files) {
			std::cout << f << std::endl;
		}
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

void ConsoleClient::getFile(const std::string& name) {
	try {
		if (!isConnected()) {
			std::cout << "Not connected" << std::endl;
			return;
		}
		auto start = std::chrono::steady_clock::now();
		client->getFile(name);
		auto end = std::chrono::steady_clock::now();
		std::cout
		    << "Download completed in "
		    << std::chrono::duration_cast<std::chrono::milliseconds>(
			   end - start)
			   .count()
		    << " ms" << std::endl;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

bool ConsoleClient::isConnected() {
	return client.operator bool();
}

const char* ConsoleClient::commands = {
    "connect <ip> <port> - connect to server\n"
    "disconnect - disconnect from server\n"
    "list - get list of files from server\n"
    "get <filename> - get file from server\n"
    "exit - exit program\n"
    "help - show help\n"};
