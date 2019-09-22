
#include "Client.hpp"
#include "Server.hpp"

#include <future>
#include <iostream>

void run_client() {
	try {
		Client cl("127.0.0.1", 9600);
		//std::cout << cl.getFileList()[0] << std::endl;
		cl.getFile("test.txt");

	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

void run_server() {
	try {
		Server(9600);

	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

int main() {

	std::thread s(run_server);
	sleep(2);
	std::thread c(run_client);

	while (true) {
		;
	}
	s.join();
	c.join();

	return 0;
}
