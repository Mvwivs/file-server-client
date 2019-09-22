
#ifndef FILEWRITER_HPP
#define FILEWRITER_HPP

#include <fstream>
#include <mutex>
#include <string>
#include <vector>

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

#endif /* FILEWRITER_HPP */
