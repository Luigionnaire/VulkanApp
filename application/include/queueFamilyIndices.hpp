#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>


namespace QueueFamily {

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily; // for the window surface
		bool isComplete() const {
			return graphicsFamily.has_value() && presentFamily.has_value(); // std::optional functionality
		}
	};

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); // get the number of queue families

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data()); // get the queue families

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, 0, surface, &presentSupport); // check if the device supports the surface
		// loop through the queue families
		int i = 0;
		for (const auto& queueFamily : queueFamilies) { // loop through the queue families
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
	
}
