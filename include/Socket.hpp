
#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <arpa/inet.h>
#include <array>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

// Реализует работу с сетью
class Socket {
protected:
	int sock;		   // сокет
	struct sockaddr_in server; // адрес соединения

	static const std::size_t READ_BUFFER_SIZE; // Размер буфера для чтения

public:
	Socket();
	virtual ~Socket();

	Socket(const std::string& address, int port);
	Socket(int newSocket, struct sockaddr_in newServer);

	// Выполняет создание нового соединения к тому же серверу
	Socket newConnection() const;

	// Возвращает текущий сокет
	int getSock() const;

	// Выполняет закрытие сокета
	void closeSocket();

	// Выполняет отправку данных
	void sendAllData(const char* buf, std::size_t len) const;

	// Выполняет чтение данных
	std::vector<char> readAllData(std::size_t len) const;
	
};

// Реализует сокет для сервера с возможностью прослушивания порта
class ServerSocket : public Socket {
private:
public:
	ServerSocket();
	ServerSocket(int port);
	~ServerSocket();

	// Подготавливает порт к прослушиванию
	void listenSocket(std::size_t queueLength) const;

	// Ожидает присоединения клиентов
	Socket acceptSocket() const;
};

// Исключение, возникающее при отключении хоста
class HostDisconnectedException : public std::exception {
	std::string what_msg;

public:
	HostDisconnectedException() : what_msg("Host disconnected") {
	}
	const char* what() {
		return what_msg.c_str();
	}
};

#endif /* SOCKET_HPP */
