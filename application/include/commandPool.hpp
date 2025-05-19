#pragma once

#include <vulkan/vulkan.h>
#include "config.hpp"
#include "queueFamilyIndices.hpp"

class CommandPool {
public:
	CommandPool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) : m_device(device), m_physicalDevice(physicalDevice), m_surface(surface) {
		createCommandPool();
		createCommandBuffers();
	}

	void createCommandPool() {
		QueueFamily::QueueFamilyIndices queueFamilyIndices = QueueFamily::findQueueFamilies(m_physicalDevice, m_surface);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // allows individual command reset
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}
	void createCommandBuffers() {
		m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // choose between primary and secondary
		allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

		if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer!");
		}
	}


	VkCommandBuffer& getCommandBuffer(uint32_t index) {
		if (index >= m_commandBuffers.size()) {
			throw std::out_of_range("Index out of range");
		}
		return m_commandBuffers[index];
	};

	VkCommandPool getCommandPool() {
		return m_commandPool;
	};
private:
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkSurfaceKHR m_surface;
	uint32_t m_queueFamilyIndex;
	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;
};