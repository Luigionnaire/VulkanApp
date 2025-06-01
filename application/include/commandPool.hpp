#pragma once
#include <vulkan/vulkan.h>
#include <vector>

/**
 * @class CommandPool
 * @brief Manages Vulkan command pool and command buffers.
 */
class CommandPool {

public:
	/**
 * @brief Constructs a CommandPool object.
 * @param device Logical Vulkan device.
 * @param physicalDevice Physical Vulkan device.
 * @param surface Vulkan rendering surface (used to find queue family index).
 */
	CommandPool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	void createCommandPool();
	void createCommandBuffers();
	/**
 * @brief Retrieves a specific command buffer by index.
 * @param index Index of the command buffer to retrieve.
 * @return Reference to the Vulkan command buffer.
 */
	VkCommandBuffer& getCommandBuffer(uint32_t index);
	VkCommandPool getCommandPool();

private:
	VkDevice m_device;                           ///< Logical Vulkan device.
	VkPhysicalDevice m_physicalDevice;           ///< Physical Vulkan device.
	VkSurfaceKHR m_surface;                      ///< Rendering surface (used to determine queue family).
	uint32_t m_queueFamilyIndex;                 ///< Graphics queue family index.
	VkCommandPool m_commandPool;                 ///< Vulkan command pool.
	std::vector<VkCommandBuffer> m_commandBuffers; ///< Command buffers allocated from the pool.
};