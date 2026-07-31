#pragma once
#define UNICODE // so windows knows what to take
#define VK_USE_PLATFORM_WIN32_KHR // so vulkan knows im using win32 instead of anything else like a normal person


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

	InitDebugConsole();
	Window window = Window(hInstance, nCmdShow, wAPPLICATION_TITLE);
	//VulkanContext context(window, ", 1, 0, 0);
	window.loop();

	

	return 0;
}



