
#ifndef FILEACCESS_HPP
#define FILEACCESS_HPP

#include <fstream>
#include <mutex>
#include <string>
#include <vector>
#include <experimental/filesystem>

class FileWriter {
private:
	std::mutex mtx;
	std::ofstream file;
	std::vector<std::streampos> locations;

public:
	FileWriter(const std::string& fileName, std::size_t count,
		     std::size_t fileSize);
	~FileWriter();
	void write(std::size_t id, const std::vector<char>& data);

private:
	FileWriter();
};

class FileReader {
private:
	std::ifstream file;
	std::mutex mtx;
	std::vector<std::streampos> locations;

public:
	FileReader(const std::string& fileName, std::size_t count,
		   std::size_t fileSize);
	~FileReader();

	void read(std::size_t id, char* data, std::size_t len);

private:
	FileReader();
};

bool isFileExists(const std::string& fileName);
std::string getFileList();
std::size_t getFileSize(const std::string& filename);

#endif /* FILEACCESS_HPP */
