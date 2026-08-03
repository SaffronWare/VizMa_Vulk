#pragma once

#include <fstream>
#include <string>
#include <stdexcept>
#include <filesystem>

#include "DEBUG.h"

namespace FileUtils
{
	[[nodiscard]]
	std::string readFile(const std::filesystem::path& path);
}