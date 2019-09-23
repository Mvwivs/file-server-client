
#include "Client.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Реализует консольный файловый клиент
class ConsoleClient {
private:
	std::unique_ptr<Client> client;	// Клиент

public:
	ConsoleClient();
	~ConsoleClient();

	// Выполняет считывание команд из консоли
	void readCommands();

private:
	// Выполняет присоединение к серверу
	void connect(const std::string& address, int port);

	// Выполняет отсоединение от сервера
	void disconnect();

	// Выполяет получение списка файлов
	void getFileList();

	// Выполяет получение файла
	void getFile(const std::string& name);

	// Выполяет проверку наличия подключения
	bool isConnected();

	static const char* commands;	// Список комманд
};
