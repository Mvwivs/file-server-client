
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
		else if (command == "download") {
			std::cin >> parameter1;
			getFile(parameter1);
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
		client->getFile(name);
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	std::cout << "Download completed" << std::endl;
}
