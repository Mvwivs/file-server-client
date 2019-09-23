
#ifndef SERVER_HPP
#define SERVER_HPP

#include "FileAccess.hpp"
#include "Message.hpp"
#include "Socket.hpp"

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <vector>

// Хранит информацию о клиенте
struct UserProcesser {
	std::thread thread;
	Socket commandSocket;
	std::vector<Socket> sockets;
};

// Реализует файловый сервер
class Server {
private:
	std::mutex mtx;
	std::condition_variable condVariable; // Оповещает потоки о присоединении клиента
	ServerSocket masterSock;	      // Слушающий сокет сервера
	std::unordered_map<std::size_t, UserProcesser> connections; // Список текущих сессий
	std::vector<std::size_t> endedSessions; // Закоченные сессии, подлежащие закрытию
	std::size_t sessionCounter;		// Счётчик сессий

	const std::size_t QUEUE_LENGTH = 5;     // Длина очереди при прослушивании
	const std::size_t FILE_CONNECTIONS = 4; // Максимальное число соединений для передачи файлов на клиента
	const std::size_t BUFFER_SIZE = 1024;   // Размер буффера для чтения из файла

public:
	Server(int port);
	~Server();

	// Запускает прослушивание сервера
	void startListening();

private:
	Server();

	// Выполняет обслуживание клиента
	void serveClient(const Socket& clienSocket, std::size_t client);

	// Выполняет оправку файла 
	void sendFile(const std::string& filename, std::size_t connCount, std::size_t filesize,
		      std::size_t client);

	// Выполняет создание сесси
	std::size_t createSession(const Socket& sock);

	// Выполняет удаление законченных сессий
	void clearEndedSessions();
};

#endif /* SERVER_HPP */
