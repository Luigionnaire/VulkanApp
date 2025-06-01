#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

/**
 * @namespace QueueFamily
 * @brief Utility functions and structures for querying Vulkan queue families.
 */
namespace QueueFamily {
	/**
 * @struct QueueFamilyIndices
 * @brief Holds indices for graphics and presentation queue families.
 */
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily; // for the window surface
		/**
 * @brief Checks if both graphics and presentation families have been found.
 * @return True if both are available, false otherwise.
 */
		bool isComplete() const {
			return graphicsFamily.has_value() && presentFamily.has_value(); // std::optional functionality
		}
	};
	/**
 * @brief Finds queue families supporting graphics and presentation for a given device and surface.
 *
 * @param device The physical device to query.
 * @param surface The window surface to check presentation support.
 * @return QueueFamilyIndices containing optional indices of the required queue families.
 */
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); // get the number of queue families

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data()); // get the queue families

		// loop through the queue families
		int i = 0;
		for (const auto& queueFamily : queueFamilies) { // loop through the queue families
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i; // set the present family
			}
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { // check if the family supports graphics
				indices.graphicsFamily = i; // set the graphics family
			}
			if (indices.isComplete()) {
				break;
			}
			i++;
		}
		return indices;
	}
	
}// namespace QueueFamily
