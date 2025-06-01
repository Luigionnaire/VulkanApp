#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "validationLayersConfig.hpp"
#include <set>
#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
/**
 * @namespace Extensions
 * @brief Contains utility functions for handling Vulkan instance and device extensions.
 */
namespace Extensions {
	/**
	* @brief Retrieves the required Vulkan instance extensions.
	*
	* Includes GLFW-required extensions and the debug utils extension if validation layers are enabled.
	*
	* @return A vector of required instance extension names.
	*/
	static std::vector<const char*> getRequiredExtensions() { // debug messenger extension  
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (ValidationLayersConfig::enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // add debug utils extension
		}
		return extensions;
	}
	/**
	 * @brief Checks if a physical device supports all required device extensions.
	 *
	 * @param device The Vulkan physical device to check.
	 * @param deviceExtensions A list of required device extensions.
	 * @return true if all required extensions are supported; false otherwise.
	 */
	static bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> deviceExtensions) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty(); // if the set is empty, all required extensions are supported
	}
}