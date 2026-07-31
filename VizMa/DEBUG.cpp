#include "DEBUG.h"

void InitDebugConsole()
{
#ifdef DEBUG
	AllocConsole();
	FILE* _ = nullptr;
	if (freopen("CONOUT$", "w", stdout) == NULL)
	{
		throw std::runtime_error("FAILED TO SET STDOUT STREAM\n");
	}
	if (freopen("CONIN$", "r", stdin) == NULL)
	{
		throw std::runtime_error("FAILED TO SET STDIN STREAM\n");
	}
	if (freopen("CONOUT$", "w", stderr) == NULL)
	{
		throw std::runtime_error("FAILED TO SET STDCERR STREAM\n");
	}

	LOG("testing console cout\n");
	CERR("testing console cerr\n");
#endif
}