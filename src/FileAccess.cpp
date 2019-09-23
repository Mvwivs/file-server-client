
#include "FileAccess.hpp"

FileWriter::FileWriter() {
}

FileWriter::~FileWriter() {
	if (file.is_open()) {
		file.close();
	}
}

FileWriter::FileWriter(const std::string& fileName, std::size_t count, std::size_t fileSize) {
	file.open(fileName, std::fstream::out | std::fstream::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Unable to create file");
	}
	locations.reserve(count);
	std::streampos pos = file.tellp();	// Выставляет на начало файла
	std::size_t offset = fileSize / count;	// Сдвиг для каждого потока относительно предыдущего
	for (std::size_t i = 0; i < count; ++i) {
		locations.push_back(pos);
		pos = file.seekp(offset, std::ios::cur).tellp();	// Сдвиг указателя для следующего потока
	}
}

void FileWriter::write(std::size_t id, const std::vector<char>& data) {
	std::lock_guard<std::mutex> lock(mtx);
	std::streampos currPos = locations[id];	// Получение точки для записи
	file.seekp(currPos);
	file.write(data.data(), data.size());
	locations[id] = file.tellp();	// Сохранение точки для записи
}

FileReader::FileReader() {
}

FileReader::~FileReader() {
	if (file.is_open()) {
		file.close();
	}
}

FileReader::FileReader(const std::string& fileName, std::size_t count, std::size_t fileSize) {
	file.open(fileName, std::fstream::in | std::fstream::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Unable to create file");
	}
	locations.reserve(count);
	std::streampos pos = file.tellg();	// Выставляет на начало файла
	std::size_t offset = fileSize / count;	// Сдвиг для каждого потока относительно предыдущего
	for (std::size_t i = 0; i < count; ++i) {
		locations.push_back(pos);
		pos = file.seekg(offset, std::ios::cur).tellg();	// Сдвиг указателя для следующего потока
	}
}

void FileReader::read(std::size_t id, char* data, std::size_t len) {
	std::lock_guard<std::mutex> lck(mtx);
	std::streampos currPos = locations[id];	// Получение точки для записи
	file.seekg(currPos);
	file.read(data, len);
	locations[id] = file.tellg();	// Сохранение точки для записи
}

bool isFileExists(const std::string& fileName) {
	std::ifstream f(fileName);
	return f.good();
}

std::size_t getFileSize(const std::string& filename) {
	namespace fs = std::experimental::filesystem;
	return fs::file_size(filename);
}

std::string getFileList() {
	namespace fs = std::experimental::filesystem;
	std::stringstream fileList;
	for (const auto& entry : fs::directory_iterator(".")) {
		if (entry.status().type() != fs::file_type::directory) {
			fileList << entry.path() << '\n';
		}
	}
	return fileList.str();
}
