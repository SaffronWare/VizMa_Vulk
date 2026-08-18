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
#include <ArkMat.hpp>

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

struct CameraUBO
{
	ark::Vec4 front; // last comp is focal
	ark::Vec4 right; // last comp is aspect
	ark::Vec4 up; // last comp is padding
	ark::Vec4 pos; // last comp is badding for 16 byte alignement
};


class VulkanContext
{
public:
	inline static GLSLangContext& glslLangContext = GLSLangContext::Get();

	VulkanContext(const Window& window, const char* title = "My Application", int version_major = 1, int version_minor = 0, int sub_ver = 0);
	~VulkanContext();

	void drawFrame();

private:

	CameraUBO CamUBO;

	void PopulateAppInfo(const char* title, int version_major, int version_minor, int sub_ver);
	void CreateInstance();
	void CreateWindowSurfaceWin32();
	void GetPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapchain();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();
	void CreateFrameBuffers();
	void CreateCommandPool();
	void CreateUniformBuffers();
	void CreateCommandBuffer();
	void CreateSyncObjects();
	void RecreateSwapchain();
	void CleanupSwapchain();

	void recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex) const;
	void updateUniformBuffer() const;

	bool IsPhysicalDeviceValid(const VkPhysicalDevice& device) const;
	bool IsCompletePhysicalDeviceExtensions(const VkPhysicalDevice& device) const;
	int ScorePhysicalDevice(const VkPhysicalDevice& device) const;

	uint32_t vkFindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
 
	const Window& vkWin32Window;

	QueueFamilyIndices findPhysicalDeviceQueueFamilies(const VkPhysicalDevice& device) const;
	SwapchainSupportDetails querySwapchainSupportDetails(const VkPhysicalDevice& device) const;

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) const;
	
	VkShaderModule createShaderModule(const std::vector<uint32_t>& code) const;

	VkApplicationInfo appInfo{};

	VkInstance vkInstance = VK_NULL_HANDLE;
	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
	VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
	VkDevice vkLogicalDevice = VK_NULL_HANDLE;
	VkQueue vkGraphicsQueue = VK_NULL_HANDLE;
	VkQueue vkPresentQueue = VK_NULL_HANDLE;
	VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
	VkRenderPass vkRenderPass = VK_NULL_HANDLE;
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
	VkPipeline vkGraphicsPipeline = VK_NULL_HANDLE;
	VkCommandPool vkCommandPool = VK_NULL_HANDLE;
	VkCommandBuffer vkCommandBuffer = VK_NULL_HANDLE;
	VkSemaphore vkImageReadySemaphore = VK_NULL_HANDLE;
	VkSemaphore vkRenderFinishedSemaphore = VK_NULL_HANDLE;
	VkFence vkInFlightFence = VK_NULL_HANDLE;


	std::vector<const char*> vkRequiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};


	std::vector<VkImage> vkSwapchainImages;
	std::vector<VkImageView> vkSwapchainImageViews;
	std::vector<VkFramebuffer> vkSwapchainFrameBuffers;

	VkBuffer vkCameraUbo;
	VkDeviceMemory vkCameraDevMemory;
	void* vkCameraUboMemMapped;

	VkSurfaceFormatKHR vkSurfaceFormat;
	VkExtent2D vkSurfaceExtent;
	VkPresentModeKHR vkSurfacePresentMode;
};