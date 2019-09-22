
#include "FileWriter.hpp"

FileWriter::FileWriter() {
}

FileWriter::~FileWriter() {
	if (file.is_open()) {
		file.close();
	}
}

FileWriter::FileWriter(const std::string& fileName, std::size_t count, std::size_t fileSize) {
	//file.open(fileName, std::fstream::out | std::fstream::binary);
	file.open("test_recieved.txt", std::fstream::out | std::fstream::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Unable to create file");
	}
	locations.reserve(count);
	std::streampos pos = file.tellp();
	std::size_t offset = fileSize / count;
	for (std::size_t i = 0; i < count; ++i) {
		locations.push_back(pos);
		pos = file.seekp(offset, std::ios::cur).tellp();
	}
}

void FileWriter::write(std::size_t id, const std::vector<char>& data) {
	std::lock_guard<std::mutex> lock(mtx);
	std::streampos currPos = locations[id];
	file.seekp(currPos);
	file.write(data.data(), data.size());
	locations[id] = file.tellp();
}