#include "DEBUG.h"

#ifdef VIZMA_DEBUG
DebugModeInitializer DebugInitializer;
#endif

void InitDebugConsole()
{
#ifdef VIZMA_DEBUG
	AllocConsole();
	FILE* _ = nullptr;
	if (freopen_s(&_, "CONIN$", "r", stdin) != 0)
	{
		throw std::runtime_error("FAILED TO SET STDIN STREAM\n");
	}
	if (freopen_s(&_, "CONOUT$", "w", stdout) != 0)
	{
		throw std::runtime_error("FAILED TO SET STDOUT STREAM\n");
	}
	if (freopen_s(&_, "CONOUT$", "w", stderr) != 0)
	{
		throw std::runtime_error("FAILED TO SET STDCERR STREAM\n");
	}

	std::cout.clear();
	std::cin.clear();
	std::cerr.clear();

	LOG("testing console cout\n");
	CERR("testing console cerr\n");
	CERR("testing console cout again\n");
#endif
}