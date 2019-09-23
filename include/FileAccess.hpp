
#ifndef FILEACCESS_HPP
#define FILEACCESS_HPP

#include <experimental/filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

// Реализует многопоточную запись в файл
class FileWriter {
private:
	std::mutex mtx;
	std::ofstream file;
	std::vector<std::streampos> locations; // Точки в файле для каждого потока, в которые они пишут

public:
	// Выполняет инициализацию, count - число потоков
	FileWriter(const std::string& fileName, std::size_t count, std::size_t fileSize);
	~FileWriter();

	// Выполняет запись в файл, id - номер потока
	void write(std::size_t id, const std::vector<char>& data);

private:
	FileWriter();
};

// Реализует многопоточное чтение из файла
class FileReader {
private:
	std::ifstream file;
	std::mutex mtx;
	std::vector<std::streampos> locations; // Точки в файле для каждого потока, из которых они читают

public:
	// Выполяет инициализацию, count - число потоков
	FileReader(const std::string& fileName, std::size_t count, std::size_t fileSize);
	~FileReader();

	// Выполняет чтение из файла, id - номер потока
	void read(std::size_t id, char* data, std::size_t len);

private:
	FileReader();
};

// Выплняет проверку существования файла в текущей директории
bool isFileExists(const std::string& fileName);

// Выполняет получение списка файлов в текущей директории в виде одной строки, разделённой переносами
std::string getFileList();

// Получает размер файла
std::size_t getFileSize(const std::string& filename);

#endif /* FILEACCESS_HPP */
