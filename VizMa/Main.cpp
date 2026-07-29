#pragma once
#define UNICODE // so windows knows what to take
#define VK_USE_PLATFORM_WIN32_KHR // so vulkan knows im using win32 instead of anything else like a normal person
#define DEBUG

#ifdef DEBUG
#define LOG(x) std::cout << x << std::endl
#define CERR(x) std::cerr << x << std::endl
#else
#define LOG(x)
#define CERR(x)
#endif

#include "Window.h"
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>

const wchar_t* APPLICATION_TITLE = L"VizMa";


void OnSize(HWND hwnd, UINT flag, int width, int height)
{

}

bool vkDeviceIsValid(VkPhysicalDevice& _vkDevice)
{
	// proprties = name,type, supported vulkan version
	VkPhysicalDeviceProperties vkDeviceProperties;
	vkGetPhysicalDeviceProperties(_vkDevice, &vkDeviceProperties);

	// gives support details for optional features
	VkPhysicalDeviceFeatures vkDeviceFeatures;
	vkGetPhysicalDeviceFeatures(_vkDevice, &vkDeviceFeatures);

	LOG((vkDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU));

	return (vkDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
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
	
	Window window = Window(hInstance, nCmdShow, APPLICATION_TITLE);

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
	vkSurfaceCreateInfo.hwnd = window.getWindowHandle();
	// altough hwnd is enough to identify window,
	// because win32 needs hInstance, classname to identify classes, vulkan mirrors this
	vkSurfaceCreateInfo.hinstance = hInstance;

	// the gpu were communicating
	VkPhysicalDevice vkDevice = VK_NULL_HANDLE;
	uint32_t vkNumDevices = 0;
	// first get the number of devices
	vkEnumeratePhysicalDevices(vkInstance, &vkNumDevices, nullptr);

	// if no devices somethings going fucking wrong and were running ona  ti84
	if (vkNumDevices == 0)
	{
		throw std::runtime_error("No physical devices detected!\n");
	}

	std::vector<VkPhysicalDevice> vkDevices(vkNumDevices);

	// next fill an array with the devices
	vkEnumeratePhysicalDevices(vkInstance, &vkNumDevices, vkDevices.data());
	
	for (VkPhysicalDevice& _vkDevice : vkDevices)
	{
		LOG(_vkDevice);

		if (vkDeviceIsValid(_vkDevice))
		{
			// do some stuff
			vkDevice = _vkDevice;
			break;
		}
	}

	if (vkDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("No valid physical device selected!\n");
	}





	

	// this is more of a contact surface to get info about the window (pixel support, drawing support etc..)
	VkSurfaceKHR vkSurface;
	VkResult vkSurfaceCreationResult = vkCreateWin32SurfaceKHR(vkInstance, &vkSurfaceCreateInfo, nullptr, &vkSurface);
	if (vkSurfaceCreationResult != VK_SUCCESS)
	{
		throw std::runtime_error("Couldn't create vkSurface\n");
	}
	
	
	window.loop();

	// cleanup
	vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
	vkDestroyInstance(vkInstance, nullptr);

	return 0;
}



