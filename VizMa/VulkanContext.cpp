#include "VulkanContext.h"



bool QueueFamilyIndices::isComplete()
{
	return graphicsFamily.has_value() && presentFamily.has_value();
}

bool SwapchainSupportDetails::isSupported()
{
	return !vkSurfaceFormats.empty() && !vkPresentModes.empty();
}


void VulkanContext::PopulateAppInfo(const char* title, int version_major, int version_minor, int sub_ver)
{
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;
	appInfo.pApplicationName = title;
	appInfo.applicationVersion = VK_MAKE_VERSION(version_major, version_minor, sub_ver);
}


void VulkanContext::CreateInstance()
{

	// info for creating our instance
	VkInstanceCreateInfo vkInstanceCreateInfo{};
	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pApplicationInfo = &appInfo; 

	vkInstanceCreateInfo.enabledLayerCount = 0; 

	vkInstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(2);
	const char* InstanceExtensions[3] = { VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME}; 
	vkInstanceCreateInfo.ppEnabledExtensionNames = InstanceExtensions; 

	VkResult vkInstanceResult = vkCreateInstance(&vkInstanceCreateInfo, nullptr, &vkInstance);
	if (vkInstanceResult != VK_SUCCESS)
	{
		throw std::runtime_error("Coudln't create vkInstance\n");
	}
}


void VulkanContext::CreateWindowSurfaceWin32(const Window& window)
{

	VkWin32SurfaceCreateInfoKHR  vkSurfaceCreateInfo = {};
	vkSurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vkSurfaceCreateInfo.hwnd = window.getWindowHandle();

	vkSurfaceCreateInfo.hinstance = window.getWindowInstance();


	VkResult vkSurfaceCreationResult = vkCreateWin32SurfaceKHR(vkInstance, &vkSurfaceCreateInfo, nullptr, &vkSurface);
	if (vkSurfaceCreationResult != VK_SUCCESS)
	{
		throw std::runtime_error("Couldn't create vkSurface\n");
	}
}

bool VulkanContext::IsCompletePhysicalDeviceExtensions(const VkPhysicalDevice& device) const
{
	

	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> vkDeviceExtensions(extension_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, vkDeviceExtensions.data());

	std::set<std::string> extensionsToFind(std::begin(vkRequiredDeviceExtensions), std::end(vkRequiredDeviceExtensions));

	for (const auto& vkDeviceExtension : vkDeviceExtensions)
	{
		extensionsToFind.erase(std::string(vkDeviceExtension.extensionName));
	}

	return extensionsToFind.size() == 0;
}


bool VulkanContext::IsPhysicalDeviceValid(const VkPhysicalDevice& device) const {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	

	if (!IsCompletePhysicalDeviceExtensions(device) || !findPhysicalDeviceQueueFamilies(device).isComplete())
	{
		return false;
	}

	if (!QuerySwapchainSupportDetails(device).isSupported())
	{
		return false;
	}

	return  true;
}

int VulkanContext::ScorePhysicalDevice(const VkPhysicalDevice& device) const
{
	int score = 0;

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}

	return score;

}

void VulkanContext::GetPhysicalDevice()
{

	uint32_t vkNumDevices = 0;

	vkEnumeratePhysicalDevices(vkInstance, &vkNumDevices, nullptr);

	if (vkNumDevices == 0)
	{
		throw std::runtime_error("No physical devices detected!\n");
	}

	std::vector<VkPhysicalDevice> vkPhysicalDevices(vkNumDevices);


	vkEnumeratePhysicalDevices(vkInstance, &vkNumDevices, vkPhysicalDevices.data());

	std::multimap<int, VkPhysicalDevice> scoredDevices;

	for (VkPhysicalDevice& _vkPhysicalDevice : vkPhysicalDevices)
	{
		LOG(_vkPhysicalDevice);

		if (IsPhysicalDeviceValid(_vkPhysicalDevice))
		{
	
			scoredDevices.insert({ ScorePhysicalDevice(_vkPhysicalDevice), _vkPhysicalDevice });
		}
	}

	if (scoredDevices.size() > 0u)
	{
		vkPhysicalDevice = (*scoredDevices.rbegin()).second;
	}

	if (vkPhysicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("No valid physical device selected!\n");
	}
}

QueueFamilyIndices VulkanContext::findPhysicalDeviceQueueFamilies(const VkPhysicalDevice& device) const
{
	QueueFamilyIndices indices;

	uint32_t vkNumQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &vkNumQueueFamilies, nullptr);

	std::vector<VkQueueFamilyProperties> vkQueueFamilies(vkNumQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &vkNumQueueFamilies, vkQueueFamilies.data());

	int i = 0;
	for (const VkQueueFamilyProperties& vkQueueFamily : vkQueueFamilies)
	{
		if (vkQueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkSurface, &presentSupport);
		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i += 1;
	}

	return indices;
}

SwapchainSupportDetails VulkanContext::QuerySwapchainSupportDetails(const VkPhysicalDevice& device) const
{
	SwapchainSupportDetails vkSupportDetails;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkSurface, &vkSupportDetails.vkSurfaceCapabilities);

	uint32_t surfaceFormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &surfaceFormatCount, nullptr);
	vkSupportDetails.vkSurfaceFormats.resize(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &surfaceFormatCount, vkSupportDetails.vkSurfaceFormats.data());

	uint32_t presentModesCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModesCount, nullptr);
	vkSupportDetails.vkPresentModes.resize(presentModesCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModesCount, vkSupportDetails.vkPresentModes.data());

	return vkSupportDetails;
}

void VulkanContext::CreateLogicalDevice()
{
	QueueFamilyIndices indices = findPhysicalDeviceQueueFamilies(vkPhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> vkQueueCreateInfos;
	std::set<uint32_t> vkUniqueQueueIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float vkQueuePriority = 1.0f;
	for (uint32_t vkQueueIndex : vkUniqueQueueIndices)
	{
		VkDeviceQueueCreateInfo vkQueueCreateInfo{};
		vkQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		vkQueueCreateInfo.queueFamilyIndex = vkQueueIndex;
		vkQueueCreateInfo.queueCount = 1;
		vkQueueCreateInfo.pQueuePriorities = &vkQueuePriority;
		vkQueueCreateInfos.push_back(vkQueueCreateInfo);
	}

	VkPhysicalDeviceFeatures vkDeviceFeatures{};

	
	VkDeviceCreateInfo vkDeviceCreationInfo{};
	vkDeviceCreationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreationInfo.queueCreateInfoCount = static_cast<uint32_t>(vkUniqueQueueIndices.size());
	vkDeviceCreationInfo.pQueueCreateInfos = vkQueueCreateInfos.data();
	vkDeviceCreationInfo.pEnabledFeatures = &vkDeviceFeatures;
	vkDeviceCreationInfo.enabledExtensionCount = static_cast<uint32_t>(vkRequiredDeviceExtensions.size());
	vkDeviceCreationInfo.ppEnabledExtensionNames = vkRequiredDeviceExtensions.data();

	if (vkCreateDevice(vkPhysicalDevice, &vkDeviceCreationInfo, nullptr, &vkLogicalDevice) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(vkLogicalDevice, indices.graphicsFamily.value(), 0, &vkGraphicsQueue);
	vkGetDeviceQueue(vkLogicalDevice, indices.presentFamily.value(), 0, &vkPresentQueue);
}


VulkanContext::VulkanContext(const Window& window, const char* title, int version_major, int version_minor, int sub_ver)
{
	PopulateAppInfo(title, version_major, version_minor, sub_ver);
	CreateInstance();
	CreateWindowSurfaceWin32(window);
	GetPhysicalDevice();
	CreateLogicalDevice();
}

VulkanContext::~VulkanContext()
{
	vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
	vkDestroyInstance(vkInstance, nullptr);
	vkDestroyDevice(vkLogicalDevice, nullptr);

}