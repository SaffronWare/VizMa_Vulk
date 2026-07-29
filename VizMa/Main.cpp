#pragma once
#define UNICODE // so windows knows what to take
#define VK_USE_PLATFORM_WIN32_KHR // so vulkan knows im using win32 instead of anything else like a normal person
#include <windows.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <stdexcept>

// creating a class name for our window



constexpr wchar_t CLASS_NAME[] = L"this is MY CLASS!";


void OnSize(HWND hwnd, UINT flag, int width, int height)
{

}

// Callback thst DispatchMessage calls once this windowproc is assigned to the window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int height;
	int width;
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		width = LOWORD(lParam);
		height = HIWORD(lParam);
		OnSize(hwnd, (UINT)wParam, width, height);
		break;
	
	}



	// default protocal
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// hInstance is basically program id, prev instance irrelevant is garbage from old windows, 
// pCmdLine = parameters in the command used to launch app, ncmdShow=number of the show state cmd requested
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// creating our window's class. Basically a template for what protocol our window uses, what instance handle
	WNDCLASSW wc = {};
	wc.lpfnWndProc = WindowProc; // protocol used (function that defines the windows behavior)
	wc.hInstance = hInstance; // handle of applicaiton that owns the class 
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc); // tells windows this class exists and to remeber it

	HWND hwnd = CreateWindowEx(
		0, // defualt stlye
		CLASS_NAME,
		L"My Window Name!!!",
		WS_OVERLAPPEDWINDOW,

		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	

	// basically contains metadata about what version of vulkan and stuff im ussing
	// fully optional struct but stuf fliek api version can help for vulkan optimizations!
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "VizMa"; // optional as well!
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // no functional meaning, for developer 
	appInfo.pEngineName = "No Engine"; // no engine here ! 
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); // for custom eneigne, agian no functional meaning
	appInfo.apiVersion = VK_API_VERSION_1_3; // fetched from vulkan/README.md

	// this is what allows us to access and USE vulkan functions
	VkInstance vkInstance = VK_NULL_HANDLE;

	// info for creating our instance
	VkInstanceCreateInfo vkInstanceCreateInfo{};
	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pApplicationInfo = &appInfo; // verifies that im on a valid version/instance. (blocks requesting 1.4 on 1.3 loader example)
	
	vkInstanceCreateInfo.enabledLayerCount = 0; // for now no validation (debugging) layers

	vkInstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(2);
	const char* InstanceExtensions[2] = { VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME }; // needed for create win32 surface
	vkInstanceCreateInfo.ppEnabledExtensionNames = InstanceExtensions; // gives VK_KHR_win32_surface and  VK_KHR_surface

	VkResult vkInstanceResult = vkCreateInstance(&vkInstanceCreateInfo, nullptr, &vkInstance);
	if (vkInstanceResult != VK_SUCCESS)
	{
		throw std::runtime_error("Coudln't create vkInstance\n");
	}


	// info for surface creation
	VkWin32SurfaceCreateInfoKHR  vkSurfaceCreateInfo = {};
	vkSurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vkSurfaceCreateInfo.hwnd = hwnd;
	// altough hwnd is enough to identify window,
	// because win32 needs hInstance, classname to identify classes, vulkan mirrors this
	vkSurfaceCreateInfo.hinstance = hInstance;


	

	// this is more of a contact surface to get info about the window (pixel support, drawing support etc..)
	VkSurfaceKHR vkSurface;
	VkResult vkSurfaceCreationResult = vkCreateWin32SurfaceKHR(vkInstance, &vkSurfaceCreateInfo, nullptr, &vkSurface);
	if (vkSurfaceCreationResult != VK_SUCCESS)
	{
		throw std::runtime_error("Couldn't create vkSurface\n");
	}
	
	ShowWindow(hwnd, nCmdShow);
	
	MSG msg = {};
	
	// Get Message is = 0 if the message is WM_QUIT
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);

	}

	vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
	DestroyWindow(hwnd);

	return 0;
}



