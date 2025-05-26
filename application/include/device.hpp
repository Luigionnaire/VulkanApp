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
	Device(const VkInstance& instance, VkSurfaceKHR& surface) {
		m_instance = instance;
		m_surface = surface;
		pickPhysicalDevice(); // pick the physical device
		createLogicalDevice(); // create the logical device

	}
	void destroyDevice() {
		vkDestroyDevice(m_device, nullptr); // destroy the device
	}
	VkDevice getDevice() {
		return m_device; // return the device
	}
	VkPhysicalDevice getPhysicalDevice() {
		return m_physicalDevice; // return the physical device
	}
	VkQueue getGraphicsQueue() {
		return graphicsQueue; // return the graphics queue
	}
	VkQueue getPresentQueue() {
		return presentQueue; // return the present queue
	}
private:
	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr); // get the number of devices
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				m_physicalDevice = device;
				break;
			}
		}
		if (m_physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}
	void createLogicalDevice() {
		QueueFamily::QueueFamilyIndices indices = QueueFamily::findQueueFamilies(m_physicalDevice, m_surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() }; // set of unique queue families
		float queuePriority = 1.0f;

		for (uint32_t queueFamily : uniqueQueueFamilies) { // loop through the queue families
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily; // set the family index
			queueCreateInfo.queueCount = 1; // set the queue count
			queueCreateInfo.pQueuePriorities = &queuePriority; // set the priority
			queueCreateInfos.push_back(queueCreateInfo); // add to the vector
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE; // enable anisotropy 

		VkDeviceCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

		if (ValidationLayersConfig::enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayersConfig::validationLayers.size());
			createInfo.ppEnabledLayerNames = ValidationLayersConfig::validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &graphicsQueue); // implicitly destroyed
		vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &presentQueue); // implicitly destroyed
	}

	bool isDeviceSuitable(VkPhysicalDevice device) // can be used to only allow certain devices based on capabilities
	{
		QueueFamily::QueueFamilyIndices indices = QueueFamily::findQueueFamilies(device, m_surface);
		bool extensionsSupported = Extensions::checkDeviceExtensionSupport(device, m_deviceExtensions);
		bool swapChainAdequate = false;

		if (extensionsSupported) {
			SwapChain::SwapChainSupportDetails swapChainSupport = SwapChain::querySwapChainSupport(device, m_surface); // get the swap chain support
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty(); // check if the swap chain is adequate
		}
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures); // get the supported features

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	VkSurfaceKHR m_surface;
	VkInstance m_instance;
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	const std::vector<const char*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; // swapchain extension
};