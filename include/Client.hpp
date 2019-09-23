
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "FileAccess.hpp"
#include "Message.hpp"
#include "Socket.hpp"

#include <future>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Реализует файловый клиент
class Client {
private:
	Socket mainSocket;	 // Сокет для обмена командами
	std::size_t session;       // Текущая сессия
	struct sockaddr_in server; // Адрес сервера

	const std::size_t PREFFERED_CONNECTIONS = 1; // Число соединений для передачи файлов
	const std::size_t READ_BUFFER = 1024; // Размер буффера для чтения файлов

public:
	~Client();

	// Выполяет подключение к серверу
	Client(const std::string& address, int port);

	// Выполяет получение списка файлов с сервера
	std::vector<std::string> getFileList() const;

	// Выполяет получение файла с сервера
	std::string getFile(std::string fileName);

private:
	Client();
	// Выполяет создание сессии с сервером
	void createSession(const Socket& sock);

	// Выполяет чтение файла из сети
	void readFile(std::size_t connCount, const std::string& filename, std::size_t fileSize);
};

#endif /* CLIENT_HPP */
