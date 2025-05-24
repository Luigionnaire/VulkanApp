#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include "bufferUtils.hpp"

class Mesh {
public:
	Mesh() = default;
	Mesh(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices)
		: m_device(device), 
		m_physicalDevice(physicalDevice),
		m_commandPool(commandPool), 
		m_graphicsQueue(graphicsQueue), 
		m_vertices(vertices), 
		m_indices(indices) 
	{
		BufferUtils::createVertexBuffer(m_device, m_physicalDevice, m_commandPool, m_graphicsQueue, m_vertices, m_vertexBuffer, m_vertexBufferMemory);
		BufferUtils::createIndexBuffer(m_device, m_physicalDevice, m_commandPool, m_graphicsQueue, m_indices, m_indexBuffer, m_indexBufferMemory);
	}

	void freeMemory() {
		vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
		vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
		vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
		vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
	}

	void bindBuffers(VkCommandBuffer commandBuffer) {
		//BUFFERS
		VkBuffer vertexBuffers[] = { m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

	}

	void draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
	}
private:
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkCommandPool m_commandPool;
	VkQueue m_graphicsQueue;

	std::vector<Vertex> m_vertices;
	std::vector<uint16_t> m_indices;

	VkBuffer m_vertexBuffer;
	VkBuffer m_indexBuffer;

	VkDeviceMemory m_vertexBufferMemory;
	VkDeviceMemory m_indexBufferMemory;
};
