#include "VulkanContext.h"



bool QueueFamilyIndices::isComplete()
{
	return graphicsFamily.has_value() && presentFamily.has_value();
}

bool SwapchainSupportDetails::isSupported()
{
	return !vkSurfaceFormats.empty() && !vkPresentModes.empty();
}

void VulkanContext::drawFrame()
{
	updateUniformBuffer();

	vkWaitForFences(vkLogicalDevice, 1, &vkInFlightFence, VK_TRUE, UINT64_MAX);

	uint32_t vkSwapchainImageIndex;
	VkResult vkFetchImageResult = vkAcquireNextImageKHR(vkLogicalDevice, vkSwapchain, UINT64_MAX, vkImageReadySemaphore, VK_NULL_HANDLE, &vkSwapchainImageIndex);
	if (vkFetchImageResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapchain();
		return;
	}

	vkResetFences(vkLogicalDevice, 1, &vkInFlightFence);

	vkResetCommandBuffer(vkCommandBuffer, 0);

	recordCommandBuffer(vkCommandBuffer, vkSwapchainImageIndex);

	VkSubmitInfo vkCmdBufferSubInfo{};
	vkCmdBufferSubInfo.commandBufferCount = 1;
	vkCmdBufferSubInfo.pCommandBuffers = &vkCommandBuffer;
	vkCmdBufferSubInfo.waitSemaphoreCount = 1;
	vkCmdBufferSubInfo.pWaitSemaphores = &vkImageReadySemaphore;
	// want to wait before coloring
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	vkCmdBufferSubInfo.pWaitDstStageMask = &waitStage;
	vkCmdBufferSubInfo.signalSemaphoreCount = 1;
	vkCmdBufferSubInfo.pSignalSemaphores = &vkRenderFinishedSemaphore;

	if (vkQueueSubmit(vkGraphicsQueue, 1, &vkCmdBufferSubInfo, vkInFlightFence) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit to graphics queue");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &vkRenderFinishedSemaphore;
	presentInfo.pSwapchains = &vkSwapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pImageIndices = &vkSwapchainImageIndex;
	
	vkQueuePresentKHR(vkPresentQueue, &presentInfo);
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


void VulkanContext::CreateWindowSurfaceWin32()
{

	VkWin32SurfaceCreateInfoKHR  vkSurfaceCreateInfo = {};
	vkSurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vkSurfaceCreateInfo.hwnd = vkWin32Window.getWindowHandle();

	vkSurfaceCreateInfo.hinstance = vkWin32Window.getWindowInstance();


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

	if (!querySwapchainSupportDetails(device).isSupported())
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

SwapchainSupportDetails VulkanContext::querySwapchainSupportDetails(const VkPhysicalDevice& device) const
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
	std::set<uint32_t> vkUniqueQueueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float vkQueuePriority = 1.0f;
	for (uint32_t vkQueueIndex : vkUniqueQueueFamilyIndices)
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
	vkDeviceCreationInfo.queueCreateInfoCount = static_cast<uint32_t>(vkUniqueQueueFamilyIndices.size());
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

VkSurfaceFormatKHR VulkanContext::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
{
	for (const VkSurfaceFormatKHR& format : availableFormats)
	{
		if ((format.format == VK_FORMAT_R8G8B8A8_SRGB) && (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
		{
			return format;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanContext::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR; // guaranteed hehe! :D
}

VkExtent2D VulkanContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) const
{
	if (surfaceCapabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
	{
		return surfaceCapabilities.currentExtent;
	}
	else
	{
		int width, height;

		width = vkWin32Window.getClientSurfaceWidth();
		height = vkWin32Window.getClientSurfaceHeight();

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, 
			surfaceCapabilities.minImageExtent.width, 
			surfaceCapabilities.maxImageExtent.width);

		actualExtent.height = std::clamp(actualExtent.height,
			surfaceCapabilities.minImageExtent.height,
			surfaceCapabilities.maxImageExtent.height);

		LOG("WINDOW WIDTH: " << width << "\nWINDOW HIEHGT:" << height);

		return actualExtent;
	}

}

void VulkanContext::CreateSwapchain()
{
	LOG("recreating swapchain\n");
	SwapchainSupportDetails vkSwapchainDetails = querySwapchainSupportDetails(vkPhysicalDevice);

	vkSurfaceFormat = chooseSwapSurfaceFormat(vkSwapchainDetails.vkSurfaceFormats);
	vkSurfacePresentMode = chooseSwapPresentMode(vkSwapchainDetails.vkPresentModes);
	vkSurfaceExtent = chooseSwapExtent(vkSwapchainDetails.vkSurfaceCapabilities);

	uint32_t imageCount = vkSwapchainDetails.vkSurfaceCapabilities.minImageCount + 1;

	// if 0 no max
	if (vkSwapchainDetails.vkSurfaceCapabilities.maxImageCount > 0 && imageCount > vkSwapchainDetails.vkSurfaceCapabilities.maxImageCount)
	{
		imageCount = vkSwapchainDetails.vkSurfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR vkSwapchainCreationInfo{};
	vkSwapchainCreationInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkSwapchainCreationInfo.surface = vkSurface;
	vkSwapchainCreationInfo.minImageCount = imageCount;
	vkSwapchainCreationInfo.imageFormat = vkSurfaceFormat.format;
	vkSwapchainCreationInfo.imageColorSpace = vkSurfaceFormat.colorSpace;
	vkSwapchainCreationInfo.presentMode = vkSurfacePresentMode;
	vkSwapchainCreationInfo.imageExtent = vkSurfaceExtent;
	vkSwapchainCreationInfo.imageArrayLayers = 1; //stereographics stuff.. ignore
	vkSwapchainCreationInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices vkQueueIndices = findPhysicalDeviceQueueFamilies(vkPhysicalDevice);
	uint32_t vkQueueIndicesArray[2] = { vkQueueIndices.graphicsFamily.value(), vkQueueIndices.presentFamily.value() };

	if (vkQueueIndices.graphicsFamily.value() != vkQueueIndices.presentFamily.value())
	{
		vkSwapchainCreationInfo.queueFamilyIndexCount = 2;
		vkSwapchainCreationInfo.pQueueFamilyIndices = vkQueueIndicesArray;
		vkSwapchainCreationInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	}
	else
	{
		vkSwapchainCreationInfo.queueFamilyIndexCount = 0;
		vkSwapchainCreationInfo.pQueueFamilyIndices = nullptr;
		vkSwapchainCreationInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	vkSwapchainCreationInfo.preTransform = vkSwapchainDetails.vkSurfaceCapabilities.currentTransform;
	vkSwapchainCreationInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	vkSwapchainCreationInfo.presentMode = vkSurfacePresentMode;
	vkSwapchainCreationInfo.clipped = VK_TRUE;	
	vkSwapchainCreationInfo.oldSwapchain = nullptr;

	vkSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(vkLogicalDevice, &vkSwapchainCreationInfo, nullptr, &vkSwapchain) != VK_SUCCESS) {
		
		throw std::runtime_error("failed to create swap chain!");
	}

	uint32_t vkNumSwapchainImages;
	vkGetSwapchainImagesKHR(vkLogicalDevice, vkSwapchain, &vkNumSwapchainImages, nullptr);
	vkSwapchainImages.resize(vkNumSwapchainImages);
	vkGetSwapchainImagesKHR(vkLogicalDevice, vkSwapchain, &vkNumSwapchainImages, vkSwapchainImages.data());
}

void VulkanContext::CreateImageViews()
{
	vkSwapchainImageViews.resize(vkSwapchainImages.size());

	for (size_t i = 0; i < vkSwapchainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = vkSwapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = vkSurfaceFormat.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(vkLogicalDevice, &createInfo, nullptr, &vkSwapchainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	
	}
}

void VulkanContext::CreateGraphicsPipeline()
{
	GLSLShaderCompileInfo info;
	info.source = FileUtils::readFile("C:\\Users\\aryan\\source\\repos\\VizMa_Vulk\\VizMa\\Shaders\\vert.glsl");
	info.stage = EShLangVertex;
	std::vector<uint32_t> vertexSPIRV = glslLangContext.compileShader(info);

	info.source = FileUtils::readFile("C:\\Users\\aryan\\source\\repos\\VizMa_Vulk\\VizMa\\Shaders\\frag.glsl");
	info.stage = EShLangFragment;
	std::vector<uint32_t> fragmentSPIRV = glslLangContext.compileShader(info);

	VkShaderModule vkVertexShaderModule = createShaderModule(vertexSPIRV);
	VkShaderModule vkFragmentShaderModule = createShaderModule(fragmentSPIRV);

	VkPipelineShaderStageCreateInfo vkVertInfo{};
	vkVertInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkVertInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vkVertInfo.module = vkVertexShaderModule;
	vkVertInfo.pName = "main";

	VkPipelineShaderStageCreateInfo vkFragInfo{};
	vkFragInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkFragInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	vkFragInfo.module = vkFragmentShaderModule;
	vkFragInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStagesInfo[] = { vkVertInfo, vkFragInfo };


	std::vector<VkDynamicState> vkDynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo vkDynamicState{};
	vkDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	vkDynamicState.dynamicStateCount = static_cast<uint32_t>(vkDynamicStates.size());
	vkDynamicState.pDynamicStates = vkDynamicStates.data();


	VkPipelineVertexInputStateCreateInfo vkVertexInputInfo{};
	vkVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vkVertexInputInfo.vertexBindingDescriptionCount = 0; // empty cus no bindings (spacing) + attribut (data types)
	vkVertexInputInfo.pVertexBindingDescriptions = nullptr;
	vkVertexInputInfo.vertexAttributeDescriptionCount = 0;
	vkVertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo vkInputAssembly{};
	vkInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vkInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	vkInputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport vkViewport{};
	vkViewport.x = 0.0f;
	vkViewport.y = 0.0f;
	vkViewport.width = vkSurfaceExtent.width;
	vkViewport.height = vkSurfaceExtent.height;
	vkViewport.minDepth = 0.0f;
	vkViewport.maxDepth = 1.0f;

	VkRect2D vkScissor;
	vkScissor.offset = { 0,0 };
	vkScissor.extent = vkSurfaceExtent;

	VkPipelineViewportStateCreateInfo vkViewportState{};
	vkViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vkViewportState.viewportCount = 1;
	vkViewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo vkRasterizer{};
	vkRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	vkRasterizer.depthClampEnable = VK_FALSE;
	vkRasterizer.rasterizerDiscardEnable = VK_FALSE;
	vkRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	vkRasterizer.lineWidth = 1.0f;
	vkRasterizer.cullMode = VK_CULL_MODE_NONE;
	vkRasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	vkRasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState vkColorBlendAttachment{};
	vkColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	vkColorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo vkColorBlending{};
	vkColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	vkColorBlending.logicOpEnable = VK_FALSE;
	vkColorBlending.attachmentCount = 1;
	vkColorBlending.pAttachments = &vkColorBlendAttachment;
	

	VkPipelineLayoutCreateInfo vkPipelineLayoutInfo{};
	vkPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vkPipelineLayoutInfo.setLayoutCount = 1;
	vkPipelineLayoutInfo.pSetLayouts = &vkDescriptorSetLayout;

	if (vkCreatePipelineLayout(vkLogicalDevice, &vkPipelineLayoutInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout\n");
	}

	VkGraphicsPipelineCreateInfo vkPipelineInfo{};
	vkPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkPipelineInfo.stageCount = 2;
	vkPipelineInfo.pStages = shaderStagesInfo;
	vkPipelineInfo.pVertexInputState = &vkVertexInputInfo;
	vkPipelineInfo.pInputAssemblyState = &vkInputAssembly;
	vkPipelineInfo.pViewportState = &vkViewportState;
	vkPipelineInfo.pRasterizationState = &vkRasterizer;
	vkPipelineInfo.pMultisampleState = &multisampling;
	vkPipelineInfo.pColorBlendState = &vkColorBlending;
	vkPipelineInfo.pDynamicState = &vkDynamicState;
	vkPipelineInfo.layout = vkPipelineLayout;
	vkPipelineInfo.renderPass = vkRenderPass;
	vkPipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(vkLogicalDevice, nullptr, 1, &vkPipelineInfo, nullptr, &vkGraphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("pipeline creation failed\n");
	}
	
}

void VulkanContext::CreateRenderPass()
{
	VkAttachmentDescription vkColorAttachment{};
	vkColorAttachment.format = vkSurfaceFormat.format;
	vkColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	vkColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	vkColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	
	// no stenciles so we dont give an idgaf
	vkColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	vkColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	vkColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference vkColorAttachmentRef{};
	vkColorAttachmentRef.attachment = 0;
	vkColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	VkSubpassDescription vkSubpass{};
	vkSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkSubpass.colorAttachmentCount = 1;
	vkSubpass.pColorAttachments = &vkColorAttachmentRef;

	VkRenderPassCreateInfo vkRenderPassInfo{};
	vkRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	vkRenderPassInfo.attachmentCount = 1;
	vkRenderPassInfo.pAttachments = &vkColorAttachment;
	vkRenderPassInfo.subpassCount = 1;
	vkRenderPassInfo.pSubpasses = &vkSubpass;
	vkRenderPassInfo.dependencyCount = 1;
	vkRenderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(vkLogicalDevice, &vkRenderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void VulkanContext::CreateFrameBuffers()
{
	vkSwapchainFrameBuffers.resize(vkSwapchainImages.size());

	for (size_t i = 0; i < vkSwapchainFrameBuffers.size(); i++)
	{
		VkImageView vkImageViewAttachments[] = {
			vkSwapchainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = vkRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = vkImageViewAttachments;
		framebufferInfo.width = vkSurfaceExtent.width;
		framebufferInfo.height = vkSurfaceExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(vkLogicalDevice, &framebufferInfo, nullptr, &vkSwapchainFrameBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create fame buffer!\n");
		}
	}
}

void VulkanContext::CreateCommandPool()
{
	QueueFamilyIndices vkQueueIndices = findPhysicalDeviceQueueFamilies(vkPhysicalDevice);
	
	VkCommandPoolCreateInfo vkCommandPoolInfo{};
	vkCommandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkCommandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vkCommandPoolInfo.queueFamilyIndex = vkQueueIndices.graphicsFamily.value();

	if (vkCreateCommandPool(vkLogicalDevice, &vkCommandPoolInfo, nullptr, &vkCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!\n");
	}

}

void VulkanContext::CreateCommandBuffer()
{
	VkCommandBufferAllocateInfo vkCmdBufferInfo{};
	vkCmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCmdBufferInfo.commandBufferCount = 1;
	vkCmdBufferInfo.commandPool = vkCommandPool;
	vkCmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (vkAllocateCommandBuffers(vkLogicalDevice, &vkCmdBufferInfo, &vkCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command buffer!");
	}
}

void VulkanContext::recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex) const
{
	VkCommandBufferBeginInfo vkBeginInfo{};
	vkBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	
	if (vkBeginCommandBuffer(buffer, &vkBeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to beign command buffer!");
	}

	VkRenderPassBeginInfo vkRenderBeginInfo{};
	vkRenderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	vkRenderBeginInfo.framebuffer = vkSwapchainFrameBuffers[imageIndex];
	vkRenderBeginInfo.renderPass = vkRenderPass;
	vkRenderBeginInfo.renderArea.offset = { 0,0 };
	vkRenderBeginInfo.renderArea.extent = vkSurfaceExtent;
	VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
	vkRenderBeginInfo.pClearValues = &clearValue;
	vkRenderBeginInfo.clearValueCount = 1;

	vkCmdBeginRenderPass(buffer, &vkRenderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkGraphicsPipeline);
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(vkSurfaceExtent.width);
	viewport.height = static_cast<float>(vkSurfaceExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(buffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = vkSurfaceExtent;
	vkCmdSetScissor(buffer, 0, 1, &scissor);

	vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet, 0, nullptr);
	vkCmdDraw(buffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(buffer);

	if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

}

void VulkanContext::CreateSyncObjects()
{
	VkSemaphoreCreateInfo vkSemaphoreInfo{};
	vkSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo vkFenceInfo{};
	vkFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;	

	if (vkCreateSemaphore(vkLogicalDevice, &vkSemaphoreInfo, nullptr, &vkImageReadySemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(vkLogicalDevice, &vkSemaphoreInfo, nullptr, &vkRenderFinishedSemaphore) != VK_SUCCESS ||
		vkCreateFence(vkLogicalDevice, &vkFenceInfo, nullptr, &vkInFlightFence) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create semaphores or fences!");
	}

}

VkShaderModule VulkanContext::createShaderModule(const std::vector<uint32_t>& code) const
{
	VkShaderModuleCreateInfo vkCreateInfo{};
	vkCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkCreateInfo.codeSize = code.size() * static_cast<size_t>(sizeof(uint32_t));
	vkCreateInfo.pCode = code.data();
	
	VkShaderModule vkShaderModule;
	if (vkCreateShaderModule(vkLogicalDevice, &vkCreateInfo, nullptr, &vkShaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to compile shader with SPIRV source");
	}

	return vkShaderModule;
}

void VulkanContext::RecreateSwapchain()
{
	vkDeviceWaitIdle(vkLogicalDevice);

	CleanupSwapchain();
	CreateSwapchain();
	CreateImageViews();
	CreateFrameBuffers();
}

void VulkanContext::CleanupSwapchain()
{
	for (VkFramebuffer& vkFrameBuffer : vkSwapchainFrameBuffers)
	{
		vkDestroyFramebuffer(vkLogicalDevice, vkFrameBuffer, nullptr);
	}

	for (VkImageView& imageView : vkSwapchainImageViews)
	{
		vkDestroyImageView(vkLogicalDevice, imageView, nullptr);
	}

	vkDestroySwapchainKHR(vkLogicalDevice, vkSwapchain, nullptr);

}



void VulkanContext::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding vkCameraUboBinding{};
	vkCameraUboBinding.binding = 0;
	vkCameraUboBinding.descriptorCount = 1;
	vkCameraUboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vkCameraUboBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo{};
	vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkDescriptorSetLayoutCreateInfo.bindingCount = 1;
	vkDescriptorSetLayoutCreateInfo.pBindings = &vkCameraUboBinding;

	if (vkCreateDescriptorSetLayout(vkLogicalDevice, &vkDescriptorSetLayoutCreateInfo, nullptr, &vkDescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout!\n");
	}
}

uint32_t VulkanContext::vkFindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
	VkPhysicalDeviceMemoryProperties vkDevMemProperties;
	vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &vkDevMemProperties);

	for (uint32_t i = 0; i < vkDevMemProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && ((vkDevMemProperties.memoryTypes[i].propertyFlags & properties) == properties))
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type! \n");
}

void VulkanContext::CreateUniformBuffers()
{
	VkDeviceSize vkUboSize = sizeof(CameraDataContainer);

	VkBufferCreateInfo vkUboBufferCreateInfo{};
	vkUboBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkUboBufferCreateInfo.size = vkUboSize;
	vkUboBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	vkUboBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vkLogicalDevice, &vkUboBufferCreateInfo, nullptr, &vkCameraUbo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create camera UBo buffer\N");
	}

	VkMemoryRequirements vkUboMemRequirements;
	vkGetBufferMemoryRequirements(vkLogicalDevice, vkCameraUbo, &vkUboMemRequirements);

	VkMemoryAllocateInfo vkUboMemAllocInfo{};
	vkUboMemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkUboMemAllocInfo.allocationSize = vkUboMemRequirements.size;
	vkUboMemAllocInfo.memoryTypeIndex = vkFindMemoryType(vkUboMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(vkLogicalDevice, &vkUboMemAllocInfo, nullptr, &vkCameraDevMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("faileld to allocate memory for camera ubo\n");
	}

	vkBindBufferMemory(vkLogicalDevice, vkCameraUbo, vkCameraDevMemory, 0);

	vkMapMemory(vkLogicalDevice, vkCameraDevMemory, 0, vkUboSize, 0, &vkCameraUboMemMapped);
}

void VulkanContext::updateUniformBuffer()
{
	static auto last_time = std::chrono::high_resolution_clock::now();

	auto current_time = std::chrono::high_resolution_clock::now();
	float dt = std::chrono::duration<float, std::chrono::seconds::period>(current_time - last_time).count();

	if (GetAsyncKeyState('W') & 0x8000)
	{
		Cam.updPos(Cam.getFront() * dt);
	}

	if (GetAsyncKeyState('S') & 0x8000)
	{
		Cam.updPos(Cam.getFront() * dt * -1.0f);
	}

	if (GetAsyncKeyState('D') & 0x8000)
	{
		Cam.updPos(Cam.getRight() * dt);
	}

	if (GetAsyncKeyState('A') & 0x8000)
	{
		Cam.updPos(Cam.getRight() * dt * -1.0f);
	}

	if (GetAsyncKeyState('E') & 0x8000)
	{
		Cam.updPos(Cam.getUp() * dt);
	}

	if (GetAsyncKeyState('Q') & 0x8000)
	{
		Cam.updPos(Cam.getUp() * dt * -1.0f);
	}

	if (GetAsyncKeyState(ARW_RIGHT) & 0x8000)
	{
		Cam.orientation.y += 0.1 * dt;
	}

	if (GetAsyncKeyState(ARW_LEFT) & 0x8000)
	{
		Cam.orientation.y -= 0.1 * dt;
	}

	Cam.update();
	Cam.writeData(vkCameraUboMemMapped);

	last_time = std::chrono::high_resolution_clock::now();
}

void VulkanContext::CreateDescriptorPool()
{
	VkDescriptorPoolSize vkPoolSize{};
	vkPoolSize.descriptorCount = 1;
	vkPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorPoolCreateInfo vkPoolCreateInfo{};
	vkPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	vkPoolCreateInfo.maxSets = 1;
	vkPoolCreateInfo.poolSizeCount = 1;
	vkPoolCreateInfo.pPoolSizes = &vkPoolSize;

	if (vkCreateDescriptorPool(vkLogicalDevice, &vkPoolCreateInfo, nullptr, &vkDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool!\n");
	}
}

void VulkanContext::CreateDescriptorSets()
{
	VkDescriptorSetAllocateInfo vkDescSetAllocInfo{};
	vkDescSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	vkDescSetAllocInfo.descriptorSetCount = 1;
	vkDescSetAllocInfo.descriptorPool = vkDescriptorPool;
	vkDescSetAllocInfo.pSetLayouts = &vkDescriptorSetLayout;
	
	if (vkAllocateDescriptorSets(vkLogicalDevice, &vkDescSetAllocInfo, &vkDescriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor sets!\n");
	}

	VkDescriptorBufferInfo vkDescBuffInfo{};
	vkDescBuffInfo.buffer = vkCameraUbo;
	vkDescBuffInfo.offset = 0;
	vkDescBuffInfo.range = sizeof(CameraDataContainer);

	VkWriteDescriptorSet vkDescSetWrite{};
	vkDescSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	vkDescSetWrite.dstSet = vkDescriptorSet;
	vkDescSetWrite.dstBinding = 0;
	vkDescSetWrite.dstArrayElement = 0;
	vkDescSetWrite.descriptorCount = 1;
	vkDescSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vkDescSetWrite.pBufferInfo = &vkDescBuffInfo;

	vkUpdateDescriptorSets(vkLogicalDevice, 1, &vkDescSetWrite, 0, nullptr);
}

VulkanContext::VulkanContext(const Window& window, const char* title, int version_major, int version_minor, int sub_ver) : vkWin32Window(window)
{

	PopulateAppInfo(title, version_major, version_minor, sub_ver);
	CreateInstance();
	CreateWindowSurfaceWin32();
	GetPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapchain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateFrameBuffers();
	CreateCommandPool();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffer();
	CreateSyncObjects();
}


VulkanContext::~VulkanContext()
{
	vkDestroySemaphore(vkLogicalDevice, vkImageReadySemaphore, nullptr);
	vkDestroySemaphore(vkLogicalDevice, vkRenderFinishedSemaphore, nullptr);
	vkDestroyFence(vkLogicalDevice, vkInFlightFence, nullptr);

	vkDestroyCommandPool(vkLogicalDevice, vkCommandPool, nullptr);
	vkDestroyBuffer(vkLogicalDevice, vkCameraUbo, nullptr);
	vkFreeMemory(vkLogicalDevice, vkCameraDevMemory, nullptr);
	vkDestroyDescriptorPool(vkLogicalDevice, vkDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(vkLogicalDevice, vkDescriptorSetLayout, nullptr);


	CleanupSwapchain();
	vkDestroyPipeline(vkLogicalDevice, vkGraphicsPipeline, nullptr);
	vkDestroyRenderPass(vkLogicalDevice, vkRenderPass, nullptr);
	vkDestroyDescriptorSetLayout(vkLogicalDevice, vkDescriptorSetLayout, nullptr);
	vkDestroyPipelineLayout(vkLogicalDevice, vkPipelineLayout, nullptr);

	vkDestroyDevice(vkLogicalDevice, nullptr);
	vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
	vkDestroyInstance(vkInstance, nullptr);
}