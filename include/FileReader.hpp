
#ifndef FILEREADER_HPP
#define FILEREADER_HPP

#include <fstream>
#include <string>
#include <vector>

class FileReader {
private:
	std::ifstream file;
	std::vector<std::streampos> locations;

public:
	FileReader(const std::string& fileName, std::size_t count,
		   std::size_t fileSize);
	~FileReader();

	void read(std::size_t id, char* data, std::size_t len);

private:
	FileReader();
};

#endif /* FILEREADER_HPP */
