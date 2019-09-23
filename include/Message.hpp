
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "Socket.hpp"

#include <string>
#include <tuple>
#include <vector>

// Реализует сообщения и их преобразование для обмена коммандами между клиентом и сервером
class Message {
public:
	enum class Command {   // Возможные комманды
		ERROR = 0,     // Ошибка
		SESSION_ID,    // Идентификатор сессии
		GET_FILE_LIST, // Запрос на получение списка файлов
		GET_FILE,      // Получение файла
		FILE_LIST,     // Получение списка файлов
		OPEN_CONN,     // Открытие соединений для передачи файла
		NOT_EXISTS     // Файл не существует
	};

private:
	Command header;		// Заголовок комманды
	std::vector<char> data; // Данные комманды

public:
	Message();
	~Message();

	// Конструктор заголовочного сообщения
	Message(Command c);
	// Конструктор сообщения со списком файлов
	Message(Command c, const std::string& fileList);
	// Конструктор сообщения с номером сессии
	Message(Command c, std::size_t session);
	// Конструктор сообщения для запроса получения файла
	Message(Command c, std::size_t connCount, const std::string& fileName);
	// Конструктор сообщения для получения файла
	Message(Command c, std::size_t connCount, std::size_t fileSize);

	// Выполяет извлечение данных из сообщения получения имени файла
	std::tuple<std::size_t, std::string> getFileNameMessage() const;
	// Выполяет извлечение данных из сообщения с сессией
	std::size_t getSessionMessage() const;
	// Выполяет извлечение данных из сообщения с списком файлов
	std::string getFileListMessage() const;
	// Выполяет извлечение данных из сообщения получения файла
	std::tuple<std::size_t, std::size_t> getFileMessage() const;

	// Выполняет формирование сообщения в виде пакета
	std::vector<char> createMessage() const;
	// Выполняет получение типа сообщения
	Command getType() const;

	// Выполняет отправку сообщения
	static void sendMessage(const Message& msg, const Socket& clientSocket);
	// Выполняет получение сообщения
	static Message recieveMessage(const Socket& sock);

private:
	// Выполняет получение данных сообщения
	const char* getData() const;
	// Выполняет получение размера данных сообщения
	std::size_t getSize() const;
	// Выполняет установку данных сообщения
	void setData(const std::vector<char> newData);
	// Выполняет получение данных типа std::size_t из сообщения
	std::size_t getSizet(std::size_t pos) const;
	// Выполняет присоединение строки к сообщению
	void appendData(const std::string& str);
	// Выполняет присоединение числа к сообщению
	void appendData(std::size_t num);
};

#endif /* MESSAGE_HPP */
