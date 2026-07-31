#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>


#include "Window.h"
#include "DEBUG.h"

class VulkanContext
{
public:
	VulkanContext(const Window& window, const char* title = "My Application", int version_major = 1, int version_minor = 0, int sub_ver = 0);
	~VulkanContext();
private:
	void PopulateAppInfo(const char* title, int version_major, int version_minor, int sub_ver);
	void CreateInstance();
	void CreateWindowSurfaceWin32(const Window& window);
	void GetPhysicalDevice();

	VkInstance vkInstance = VK_NULL_HANDLE;
	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
	VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
	VkApplicationInfo appInfo{};
};