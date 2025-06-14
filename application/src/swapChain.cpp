#pragma once

#include "swapChain.hpp"
#include "queueFamilyIndices.hpp"

VKSwapChain::VKSwapChain(VkSurfaceKHR& surface, VkPhysicalDevice physicalDevice, VkDevice device, GLFWwindow* window) :
	m_device(device),
	m_physicalDevice(physicalDevice),
	m_window(window)
{
	createSwapChain(surface);
	createImageViews();
}


void VKSwapChain::cleanupSwapChain() {

	// TODO destroy image view
	vkDestroyImageView(m_device, m_depthImageView, nullptr);
	vkDestroyImage(m_device, m_depthImage, nullptr);
	vkFreeMemory(m_device, m_depthImageMemory, nullptr);

	for (size_t i = 0; i < m_swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(m_device, m_swapChainFramebuffers[i], nullptr);
	}
	for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
		vkDestroyImageView(m_device, m_swapChainImageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
}


void VKSwapChain::createFrameBuffers(VkRenderPass renderPass, VkImageView depthImageView) {
	m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
	for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 2> attachments = {
		m_swapChainImageViews[i],
		depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_swapChainExtent.width;
		framebufferInfo.height = m_swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void VKSwapChain::createSwapChain(VkSurfaceKHR surface) {
	SwapChain::SwapChainSupportDetails swapChainSupport = SwapChain::querySwapChainSupport(m_physicalDevice, surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, m_window);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamily::QueueFamilyIndices indices = QueueFamily::findQueueFamilies(m_physicalDevice, surface);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily) { // doing this for ownership reasons (lookup)
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // concurrent mode
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices; // set the indices
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // exclusive mode
		createInfo.queueFamilyIndexCount = 0; // optional
		createInfo.pQueueFamilyIndices = nullptr; // optional
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE; // ignore pixels that are obscured(like if behind another window)
	createInfo.oldSwapchain = VK_NULL_HANDLE; // resizing stuff, look up later

	if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

	// save these to member variables for later
	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;
}
void VKSwapChain::createImageViews() { // can use createImageView() instead
	m_swapChainImageViews.resize(m_swapChainImages.size());

	for (size_t i = 0; i < m_swapChainImages.size(); i++) {
		m_swapChainImageViews[i] = ImageUtils::createImageView(m_device, m_swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT); // create image views for each image in the swap chain
	}
}

// depth Buffer
void VKSwapChain::createDepthResources(VkCommandPool commandPool, VkQueue queue) {
	VkFormat depthFormat = findDepthFormat();

	ImageUtils::createImage(m_device,
		m_physicalDevice,
		m_swapChainExtent.width,
		m_swapChainExtent.height,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_depthImage,
		m_depthImageMemory
	);
	m_depthImageView = ImageUtils::createImageView(m_device, m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	BufferUtils::transitionImageLayout(m_device,
		commandPool,
		queue,
		m_depthImage, depthFormat,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL); // optional

}

VkFormat VKSwapChain::findDepthFormat()
{
	return findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkSurfaceFormatKHR VKSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { // check for the format
			return availableFormat; // return the format
		}
	}
	return availableFormats[0]; // return the first format if not found
}

VkPresentModeKHR VKSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { // check for mailbox mode
			return availablePresentMode; // return the mode
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR; // return FIFO mode if not found
}

VkExtent2D VKSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;
	}
}

//depth Buffer

VkFormat VKSwapChain::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	throw std::runtime_error("failed to find supported format!");
}

