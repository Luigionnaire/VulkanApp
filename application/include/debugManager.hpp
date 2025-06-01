#pragma once

#include <vulkan/vulkan.h>
#include "validationLayersConfig.hpp"
#include <iostream>
/**
 * @namespace DebugManager
 * @brief Provides helper functions for setting up Vulkan debug utilities and validation layers.
 */
namespace DebugManager
{ 
	/**
	 * @brief Creates a Vulkan debug messenger using the VK_EXT_debug_utils extension.
	 *
	 * @param instance The Vulkan instance.
	 * @param pCreateInfo Pointer to a VkDebugUtilsMessengerCreateInfoEXT structure.
	 * @param pAllocator Optional allocator.
	 * @param pDebugMessenger Pointer to the debug messenger handle to be filled.
	 * @return VkResult indicating success or failure.
	 */
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) { // VL
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	/**
   * @brief Destroys a Vulkan debug messenger.
   *
   * @param instance The Vulkan instance.
   * @param debugMessenger The debug messenger handle to destroy.
   * @param pAllocator Optional allocator.
   */
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) { // VL
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}
	/**
	 * @brief Callback function for Vulkan validation layer messages.
	 *
	 * @param messageSeverity Severity of the message.
	 * @param messageType Type of message.
	 * @param pCallbackData Contains the actual message text.
	 * @param pUserData User data pointer (unused).
	 * @return Always returns VK_FALSE to indicate that Vulkan should not abort the call.
	 */
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
	/**
	* @brief Populates a VkDebugUtilsMessengerCreateInfoEXT struct with desired debug settings.
	*
	* @param createInfo Reference to the struct to populate.
	*/
	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)  // VL
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}
	/**
   * @brief Sets up the Vulkan debug messenger if validation layers are enabled.
   *
   * @param instance Vulkan instance.
   * @param debugMessenger Reference to the debug messenger handle.
   * @throws std::runtime_error if setup fails.
   */
	static void setupDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger) { // VL
		if (!ValidationLayersConfig::enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo); // populate the create info struct
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger"); // throw an error
		}

	}
	/**
	* @brief Checks whether the requested validation layers are available on the system.
	*
	* @return True if all requested validation layers are supported, false otherwise.
	*/
	static bool checkValidationLayerSupport() { // VL
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr); // get the number of layers
		std::vector<VkLayerProperties> availableLayers(layerCount); // create a vector of layers
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()); // get the layers

		for (const char* layerName : ValidationLayersConfig::validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				return false;
			}
		}
		return true;
	}
}