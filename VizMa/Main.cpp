#include "DEBUG.h"
#include "Window.h"
#include "VulkanContext.h"

#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>

const char* wAPPLICATION_TITLE = "VizMa";


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	try {
		Window window = Window(hInstance, nCmdShow, wAPPLICATION_TITLE);
		VulkanContext context(window, wAPPLICATION_TITLE, 1, 0, 0);

		while (window.loop())
		{
			context.drawFrame();
		}

		MessageBoxA(
			nullptr,
			nullptr,
			"r u sure?",
			MB_OK | MB_ICONERROR
		);

		return 0;
	}
	catch (std::exception& e)
	{

		MessageBoxA(
			nullptr,
			e.what(),
			"VizMa Error",
			MB_OK | MB_ICONERROR
		);
		return -1;
	}
}



