#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include "queueFamilyIndices.hpp"
#include "swapChain.hpp"
#include <stdexcept>
#include "extensions.hpp"
#include <set>
/**
 * @class Device
 * @brief Handles Vulkan physical device selection and logical device creation.
 */
class Device {
public:
	/**
	* @brief Constructs the Device and initializes physical/logical devices.
	*
	* @param instance The Vulkan instance.
	* @param surface The rendering surface used to evaluate device compatibility.
	*/
	Device(const VkInstance& instance, VkSurfaceKHR& surface);

	/**
  * @brief Cleans up the Vulkan logical device.
  */
	void destroyDevice();

	VkDevice getDevice();
	VkPhysicalDevice getPhysicalDevice();
	VkQueue getGraphicsQueue();
	VkQueue getPresentQueue();
private:
	void pickPhysicalDevice();
	void createLogicalDevice();

	/**
	 * @brief Checks if the given physical device supports required features and extensions.
	 *
	 * @param device The physical device to check.
	 * @return true if the device is suitable, false otherwise.
	 */
	bool isDeviceSuitable(VkPhysicalDevice device); ///< This can be used to only allow certain devices based on capabilities
	

	VkSurfaceKHR m_surface; ///< The rendering surface used to evaluate device compatibility.
	VkInstance m_instance; ///< The Vulkan instance used to create the device.
	VkDevice m_device; ///< The logical Vulkan device created from the physical device.
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE; ///< The physical Vulkan device selected for rendering.
	VkQueue graphicsQueue; ///< The graphics queue used for rendering operations.
	VkQueue presentQueue; ///< The present queue used for presenting images to the surface.
	const std::vector<const char*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };  ///< List of required device extensions.
};