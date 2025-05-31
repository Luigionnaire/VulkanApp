#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "imageUtils.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>

namespace SwapChain{
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}
}


class VKSwapChain { // TODO fix inconsistencies 
public:
	VKSwapChain(VkSurfaceKHR& surface, VkPhysicalDevice physicalDevice, VkDevice device, GLFWwindow* window);
	
	void cleanupSwapChain();

	void createFrameBuffers(VkRenderPass renderPass, VkImageView depthImageView);

	void createSwapChain(VkSurfaceKHR surface);
	void createImageViews();

	VkSwapchainKHR getSwapChain() {
		return m_swapChain;
	}
	VkFormat getSwapChainImageFormat() {
		return m_swapChainImageFormat;
	}
	VkExtent2D getSwapChainExtent() {
		return m_swapChainExtent;
	}
	std::vector<VkImageView> getSwapChainImageViews() {
		return m_swapChainImageViews;
	}
	std::vector<VkFramebuffer> getSwapChainFramebuffers() {
		return m_swapChainFramebuffers;
	}

	VkImageView getDepthImageView() {
		return m_depthImageView;
	}

	// depth Buffer
	void createDepthResources(VkCommandPool commandPool, VkQueue queue);

	VkFormat findDepthFormat();
private:
	// api members
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkRenderPass m_renderPass;
	GLFWwindow* m_window;

	// swapchain members
	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_swapChainImages;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	std::vector<VkImageView> m_swapChainImageViews;
	std::vector<VkFramebuffer> m_swapChainFramebuffers;

	// depth buffering
	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

	//depth Buffer

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);




	
};