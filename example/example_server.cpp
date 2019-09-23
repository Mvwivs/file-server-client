
#include "Server.hpp"

#include <future>
#include <iostream>

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

	run_server();

	return 0;
}
