example_local:
	g++ -g -std=c++17 src/* example/main.cpp -I include/ -o bin/example_local -lpthread -lstdc++fs
