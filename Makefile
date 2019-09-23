all: server client

server:
	mkdir -p bin/server
	g++ -std=c++17 src/* example/example_server.cpp -I include/ -o bin/server/example_server -lpthread -lstdc++fs

client:
	mkdir -p bin/client
	g++ -std=c++17 src/* example/example_client.cpp -I include/ -o bin/client/example_client -lpthread -lstdc++fs

debug:
	mkdir -p bin/server
	g++ -g -std=c++17 src/* example/debug.cpp -I include/ -o bin/server/debug -lpthread -lstdc++fs -D_DEBUG
