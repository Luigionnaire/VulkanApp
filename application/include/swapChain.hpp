#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "imageUtils.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>

/**
 * @namespace SwapChain
 * @brief Utilities for querying Vulkan swap chain support details.
 */
namespace SwapChain{
	/**
	 * @struct SwapChainSupportDetails
	 * @brief Holds the details about swap chain capabilities, supported formats, and present modes.
	 */
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities{}; ///< Capabilities of the surface, such as min/max image count, extent, and supported transforms.
		std::vector<VkSurfaceFormatKHR> formats; ///< Supported surface formats, including color space and format.
		std::vector<VkPresentModeKHR> presentModes; ///< Supported present modes, such as FIFO, MAILBOX, and IMMEDIATE.
	};
	/**
	* @brief Queries the support details for a swap chain on a given physical device and surface.
	*
	* @param device The Vulkan physical device to query.
	* @param surface The Vulkan surface to query.
	* @return SwapChainSupportDetails A struct containing the capabilities, formats, and present modes supported.
	*/
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


/**
 * @class VKSwapChain
 * @brief Encapsulates Vulkan swap chain creation, image views, framebuffers, and depth resources management.
 */
class VKSwapChain { 
public:
	/**
 * @brief Constructor that initializes and creates the swap chain and image views.
 * @param surface Reference to the Vulkan surface.
 * @param physicalDevice Vulkan physical device handle.
 * @param device Vulkan logical device handle.
 * @param window Pointer to the GLFW window.
 */
	VKSwapChain(VkSurfaceKHR& surface, VkPhysicalDevice physicalDevice, VkDevice device, GLFWwindow* window);
	
	/**
	 * @brief Cleans up swap chain resources such as image views, framebuffers, depth buffer and the swap chain itself.
	 */
	void cleanupSwapChain();

	/**
	 * @brief Creates framebuffers for each swap chain image view with the given render pass and depth image view.
	 * @param renderPass The Vulkan render pass to use.
	 * @param depthImageView The depth image view to attach.
	 */
	void createFrameBuffers(VkRenderPass renderPass, VkImageView depthImageView);
	/**
	 * @brief Creates the Vulkan swap chain based on the surface and physical device capabilities.
	 * @param surface The Vulkan surface for presentation.
	 */
	void createSwapChain(VkSurfaceKHR surface);
	/**
	 * @brief Creates image views for each image in the swap chain.
	 */
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

	/**
	* @brief Creates depth buffer resources including the image, memory, and image view.
	* @param commandPool Command pool used for layout transitions.
	* @param queue Queue used for submitting commands.
	*/
	void createDepthResources(VkCommandPool commandPool, VkQueue queue);


	/**
	 * @brief Finds a suitable depth format supported by the physical device.
	 * @return The chosen VkFormat for depth buffering.
	 */
	VkFormat findDepthFormat();
private:
	// api members
	VkDevice m_device; ///< Vulkan logical device handle.
	VkPhysicalDevice m_physicalDevice; ///< Vulkan physical device handle.
	VkRenderPass m_renderPass; ///< Vulkan render pass handle.
	GLFWwindow* m_window; ///< Pointer to the GLFW window for rendering context.

	// swapchain members
	VkSwapchainKHR m_swapChain; ///< Vulkan swap chain handle.
	std::vector<VkImage> m_swapChainImages; ///< Vector of images in the swap chain.
	VkFormat m_swapChainImageFormat; ///< Format of the swap chain images. 
	VkExtent2D m_swapChainExtent; ///< Extent (resolution) of the swap chain images.
	std::vector<VkImageView> m_swapChainImageViews; ///< Vector of image views for each swap chain image.
	std::vector<VkFramebuffer> m_swapChainFramebuffers; ///< Vector of framebuffers for each swap chain image view.

	// depth buffering
	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;
	/**
	* @brief Chooses the best surface format from available formats.
	* @param availableFormats Vector of available VkSurfaceFormatKHR.
	* @return The chosen VkSurfaceFormatKHR.
	*/
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	/**
	 * @brief Chooses the best present mode from available modes.
	 * @param availablePresentModes Vector of available VkPresentModeKHR.
	 * @return The chosen VkPresentModeKHR.
	 */
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	/**
	 * @brief Chooses the swap extent (resolution) for the swap chain images.
	 * @param capabilities Surface capabilities to respect.
	 * @param window GLFW window to query framebuffer size if needed.
	 * @return VkExtent2D chosen extent.
	 */
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

	/**
	 * @brief Finds a supported format from a list of candidates with desired tiling and features.
	 * @param candidates Vector of VkFormats to consider.
	 * @param tiling Desired VkImageTiling.
	 * @param features Required VkFormatFeatureFlags.
	 * @return Supported VkFormat.
	 * @throws std::runtime_error if no supported format is found.
	 */
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);




	
};