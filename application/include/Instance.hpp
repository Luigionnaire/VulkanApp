#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <stdexcept>

class Instance {
public:
	Instance()
	{
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

		//auto extensions = getRequiredExtensions();
		//createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size()); // look into
		//createInfo.ppEnabledExtensionNames = extensions.data(); // look into

		//VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		//if (enableValidationLayers) {
		//	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		//	createInfo.ppEnabledLayerNames = validationLayers.data();

		//	populateDebugMessengerCreateInfo(debugCreateInfo);
		//	createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		//}
		//else {
		//	createInfo.enabledLayerCount = 0; // no layers
		//	createInfo.pNext = nullptr; // no next struct
		//}

		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) { // check for errors
			throw std::runtime_error("failed to create instance!"); // throw an error
		}
	};
	~Instance();// complete

	const VkInstance& getInstance() const { return m_instance; } // getter for the instance
private:
	VkInstance m_instance;
};