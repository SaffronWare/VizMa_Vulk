#include "FileUtils.h"

std::string FileUtils::readFile(const std::filesystem::path& path)
{
	std::ifstream file(path);

	if (!file)
	{
		CERR("failed to open file at " << path);
		throw std::runtime_error("Failed to open file:" + path.string());
	}

	std::ostringstream stream;
	stream << file.rdbuf();

	return stream.str();

}