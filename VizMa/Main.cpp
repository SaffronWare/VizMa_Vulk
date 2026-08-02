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

	Window window = Window(hInstance, nCmdShow, wAPPLICATION_TITLE);
	VulkanContext context(window, wAPPLICATION_TITLE, 1, 0, 0);
	window.loop();
	return 0;
}



