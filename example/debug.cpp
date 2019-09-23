
#include "Client.hpp"
#include "Server.hpp"

#include <future>
#include <iostream>

void run_client() {
	try {
		Client cl("127.0.0.1", 9600);
		std::cout << cl.getFileList()[0] << std::endl;
		cl.getFile("m.png");
		cl.getFile("test.txt");
		{
			Client cl2("127.0.0.1", 9600);
			std::cout << cl2.getFileList()[0] << std::endl;
			cl2.getFile("m.png");
			cl2.getFile("test.txt");
		}

		Client cl3("127.0.0.1", 9600);
		std::cout << cl3.getFileList()[0] << std::endl;
		cl3.getFile("m.png");
		cl3.getFile("test.txt");

	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

void run_server() {
	try {
		Server s(9600);
		std::cout << "Server started" << std::endl;
		s.startListening();

	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

int main() {

	std::thread a(run_server);
	sleep(2);
	std::thread b(run_client);
	b.join();
	a.join();
	return 0;
}
