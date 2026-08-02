#pragma once

#include <iostream>
#include <stdexcept>
#include <Windows.h>

void InitDebugConsole();

#ifdef VIZMA_DEBUG
#define LOG(x) std::cout << x << std::endl
#define CERR(x) std::cerr << x << std::endl

struct DebugModeInitializer
{
	DebugModeInitializer()
	{
		std::cout << "inittt";
		InitDebugConsole();
	}
};

extern DebugModeInitializer DebugInitializer;
#else
#define LOG(x)
#define CERR(x)
#endif





