#pragma once

#include <iostream>
#include <stdexcept>
#include <Windows.h>

#define DEBUG

#ifdef DEBUG
#define LOG(x) std::cout << x << std::endl
#define CERR(x) std::cerr << x << std::endl
#else
#define LOG(x)
#define CERR(x)
#endif


void InitDebugConsole();