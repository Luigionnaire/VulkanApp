#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include "queueFamilyIndices.hpp"
#include "swapChain.hpp"
#include <stdexcept>
#include "extensions.hpp"
#include <set>

class Device {
public:
	Device(const VkInstance& instance, VkSurfaceKHR& surface);
	void destroyDevice();
	VkDevice getDevice();
	VkPhysicalDevice getPhysicalDevice();
	VkQueue getGraphicsQueue();
	VkQueue getPresentQueue();
private:
	void pickPhysicalDevice();
	void createLogicalDevice();

	bool isDeviceSuitable(VkPhysicalDevice device); // can be used to only allow certain devices based on capabilities
	

	VkSurfaceKHR m_surface;
	VkInstance m_instance;
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	const std::vector<const char*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; // swapchain extension
};