#include "VulkanContext.h"



bool QueueFamilyIndices::isComplete()
{
	return graphicsFamily.has_value();
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
	const char* InstanceExtensions[2] = { VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME }; 
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


bool VulkanContext::IsPhysicalDeviceValid(const VkPhysicalDevice& device) const {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);



	// for now first part always true
	return true && findQueueFamilies(device).isComplete();
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

QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice device) const
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

		if (indices.isComplete())
		{
			break;
		}

		i += 1;
	}

	return indices;
}

void VulkanContext::CreateLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(vkPhysicalDevice);

	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;
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
	// cleanup
	vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
	vkDestroyInstance(vkInstance, nullptr);
}