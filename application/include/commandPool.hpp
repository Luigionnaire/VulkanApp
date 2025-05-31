#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class CommandPool {

public:
	CommandPool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	void createCommandPool();
	void createCommandBuffers();
	VkCommandBuffer& getCommandBuffer(uint32_t index);
	VkCommandPool getCommandPool();

private:
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkSurfaceKHR m_surface;
	uint32_t m_queueFamilyIndex;
	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;
};