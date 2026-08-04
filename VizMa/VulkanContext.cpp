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

		return actualExtent;
	}

}

void VulkanContext::CreateSwapchain()
{
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
	vkSwapchainCreationInfo.imageArrayLayers = 1;
	vkSwapchainCreationInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices vkQueueIndices = findPhysicalDeviceQueueFamilies(vkPhysicalDevice);
	uint32_t vkQueueIndicesArray[2] = { vkQueueIndices.graphicsFamily.value(), vkQueueIndices.presentFamily.value() };

	if (vkQueueIndices.graphicsFamily != vkQueueIndices.presentFamily)
	{
		vkSwapchainCreationInfo.queueFamilyIndexCount = 2;
		vkSwapchainCreationInfo.pQueueFamilyIndices = vkQueueIndicesArray;
		vkSwapchainCreationInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	}
	else
	{
		// ignore for now
		vkSwapchainCreationInfo.queueFamilyIndexCount = 0;
		vkSwapchainCreationInfo.pQueueFamilyIndices = nullptr;
		vkSwapchainCreationInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	vkSwapchainCreationInfo.preTransform = vkSwapchainDetails.vkSurfaceCapabilities.currentTransform;
	vkSwapchainCreationInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	vkSwapchainCreationInfo.presentMode = vkSurfacePresentMode;
	vkSwapchainCreationInfo.clipped = VK_TRUE;	
	vkSwapchainCreationInfo.oldSwapchain = nullptr;

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

	VkPipelineShaderStageCreateInfo vkVertInfo;
	vkVertInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkVertInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vkVertInfo.module = vkVertexShaderModule;
	vkVertInfo.pName = "main";

	VkPipelineShaderStageCreateInfo vkFragInfo;
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


VkShaderModule VulkanContext::createShaderModule(const std::vector<uint32_t>& code)
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
	CreateGraphicsPipeline();
	CreateFrameBuffers();
	CreateCommandPool();
}


VulkanContext::~VulkanContext()
{
	for (VkFramebuffer& vkFrameBuffer : vkSwapchainFrameBuffers)
	{
		vkDestroyFramebuffer(vkLogicalDevice, vkFrameBuffer, nullptr);
	}

	vkDestroyPipeline(vkLogicalDevice, vkGraphicsPipeline, nullptr);
	vkDestroyRenderPass(vkLogicalDevice, vkRenderPass, nullptr);
	vkDestroyPipelineLayout(vkLogicalDevice, vkPipelineLayout, nullptr);

	for (VkImageView& imageView : vkSwapchainImageViews)
	{
		vkDestroyImageView(vkLogicalDevice,imageView, nullptr);
	}


	vkDestroySwapchainKHR(vkLogicalDevice, vkSwapchain, nullptr);
	vkDestroyDevice(vkLogicalDevice, nullptr);
	vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
	vkDestroyInstance(vkInstance, nullptr);
	

}