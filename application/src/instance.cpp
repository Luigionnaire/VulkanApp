#pragma once

#include "instance.hpp"

VKInstance::VKInstance() {
	if (ValidationLayersConfig::enableValidationLayers && !DebugManager::checkValidationLayerSupport()) { // check for validation layers			VL
		throw std::runtime_error("validation layers requested, but not available!"); // throw an error
	}
	VkApplicationInfo appInfo{}; // optional struct to allow Vulkan to optimise for this case
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{}; // non-optional struct
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = Extensions::getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size()); // look into
	createInfo.ppEnabledExtensionNames = extensions.data(); // look into

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (ValidationLayersConfig::enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayersConfig::validationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayersConfig::validationLayers.data();

		DebugManager::populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

	}
	else {
		createInfo.enabledLayerCount = 0; // no layers
		createInfo.pNext = nullptr; // no next struct
		m_debugMessenger = nullptr; // set to null
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) { // check for errors
		throw std::runtime_error("failed to create instance!"); // throw an error
	}
	DebugManager::setupDebugMessenger(m_instance, m_debugMessenger); // setup the debug messenger

}

VKInstance::~VKInstance() {
	if (ValidationLayersConfig::enableValidationLayers) {
		DebugManager::DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr); // destroy the debug messenger
	}
	vkDestroyInstance(m_instance, nullptr); // destroy the instance

}

const VkInstance& VKInstance::getInstance() const {
	return m_instance;
}