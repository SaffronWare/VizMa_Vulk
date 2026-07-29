#pragma once
#include <vulkan.hpp>

class VulkanContext
{
public:
	VulkanContext();
	~VulkanContext();
private:
	VkInstance vkInstance = VK_NULL_HANDLE;
	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
	VkPhysicalDevice vkDevice = VK_NULL_HANDLE;
};