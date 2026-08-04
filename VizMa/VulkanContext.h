#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <map>
#include <optional>
#include <set>
#include <algorithm>
#include <string>
#include <cstdint>
#include <limits>


#include "Window.h"
#include "FileUtils.h"
#include "GLSLangContext.h"
#include "DEBUG.h"

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete();
};

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> vkSurfaceFormats = {};
	std::vector<VkPresentModeKHR> vkPresentModes = {};

	bool isSupported();
};


class VulkanContext
{
public:
	inline static GLSLangContext& glslLangContext = GLSLangContext::Get();

	VulkanContext(const Window& window, const char* title = "My Application", int version_major = 1, int version_minor = 0, int sub_ver = 0);
	~VulkanContext();


private:
	void PopulateAppInfo(const char* title, int version_major, int version_minor, int sub_ver);
	void CreateInstance();
	void CreateWindowSurfaceWin32();
	void GetPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapchain();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateFrameBuffers();
	void CreateCommandPool();

	bool IsPhysicalDeviceValid(const VkPhysicalDevice& device) const;
	bool IsCompletePhysicalDeviceExtensions(const VkPhysicalDevice& device) const;
	int ScorePhysicalDevice(const VkPhysicalDevice& device) const;

	const Window& vkWin32Window;

	QueueFamilyIndices findPhysicalDeviceQueueFamilies(const VkPhysicalDevice& device) const;
	SwapchainSupportDetails querySwapchainSupportDetails(const VkPhysicalDevice& device) const;

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) const;
	
	VkShaderModule createShaderModule(const std::vector<uint32_t>& code);

	VkApplicationInfo appInfo{};

	VkInstance vkInstance = VK_NULL_HANDLE;
	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
	VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
	VkDevice vkLogicalDevice = VK_NULL_HANDLE;
	VkQueue vkGraphicsQueue = VK_NULL_HANDLE;
	VkQueue vkPresentQueue = VK_NULL_HANDLE;
	VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
	VkRenderPass vkRenderPass = VK_NULL_HANDLE;
	VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
	VkPipeline vkGraphicsPipeline = VK_NULL_HANDLE;
	VkCommandPool vkCommandPool = VK_NULL_HANDLE;

	std::vector<const char*> vkRequiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};


	std::vector<VkImage> vkSwapchainImages;
	std::vector<VkImageView> vkSwapchainImageViews;
	std::vector<VkFramebuffer> vkSwapchainFrameBuffers;

	VkSurfaceFormatKHR vkSurfaceFormat;
	VkExtent2D vkSurfaceExtent;
	VkPresentModeKHR vkSurfacePresentMode;
};