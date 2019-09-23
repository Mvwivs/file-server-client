all: server client

server:
	g++ -g -std=c++17 src/* example/example_server.cpp -I include/ -o bin/server/example_server -lpthread -lstdc++fs

client:
	g++ -g -std=c++17 src/* example/example_client.cpp -I include/ -o bin/client/example_client -lpthread -lstdc++fs

debug:
	g++ -g -std=c++17 src/* example/debug.cpp -I include/ -o bin/server/debug -lpthread -lstdc++fs -D_DEBUG

