#pragma once
#define GLM_FORCE_RADIANS
#include "vulkan/vulkan.h"
#include <vector>
#include <glm/glm.hpp>
#include "config.hpp"
#include "bufferUtils.hpp"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

struct UBO {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class UniformBuffers {
public:
	UniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice);
	void destroyUniformBuffers();

	void updateUniformBuffer(uint32_t currentImage, VkExtent2D swapChainExtent);
	VkBuffer getUniformBuffer(uint32_t index) const;

	std::vector<VkBuffer> getUniformBuffers() const {
		return m_uniformBuffers;
	}
private:
	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	std::vector<void*> m_uniformBuffersMapped;
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	UBO ubo; // uniform buffer object

	void createUniformBuffers();
};
