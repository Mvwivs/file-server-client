
#include "Client.hpp"

#include <future>
#include <iostream>

void run_client() {
	try {
		Client cl("127.0.0.1", 9600);

	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

void run_server() {
}

int main() {

	std::async(run_server);
	std::async(run_client);

	return 0;
}
