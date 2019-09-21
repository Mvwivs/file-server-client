
#include "Client.hpp"

Client::Client() : sock(-1) {
}

Client::~Client() {
	if (sock != -1) {
		close(sock); // ?
	}
}

Client::Client(const std::string& address, int port) {
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		throw std::runtime_error("Unable to open socket");
	}
	int binAddr = inet_addr(address.c_str());
	if (binAddr == -1) {
		throw std::runtime_error("Unable to read ip addrees: wrong format or host names are not supported");
	}
	struct sockaddr_in server;
	server.sin_addr.s_addr = binAddr;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
		throw std::runtime_error("Unable to connect to server");
	}
}
