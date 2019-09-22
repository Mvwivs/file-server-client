
#include "FileReader.hpp"

FileReader::FileReader() {
}

FileReader::~FileReader() {
	if (file.is_open()) {
		file.close();
	}
}

FileReader::FileReader(const std::string& fileName, std::size_t count,
		       std::size_t fileSize) {
	file.open(fileName, std::fstream::in | std::fstream::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Unable to create file");
	}
	locations.reserve(count);
	std::streampos pos = file.tellg();
	std::size_t offset = fileSize / count;
	for (std::size_t i = 0; i < count; ++i) {
		locations.push_back(pos);
		pos = file.seekg(offset, std::ios::cur).tellg();
	}

}

void FileReader::read(std::size_t id, char* data, std::size_t len) {
	std::streampos currPos = locations[id];
	file.seekg(currPos);
	file.read(data, len);
	locations[id] = file.tellg();
}