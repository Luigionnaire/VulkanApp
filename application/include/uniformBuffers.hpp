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
	UniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice) : m_device(device) , m_physicalDevice(physicalDevice) {
		createUniformBuffers();
	}
	void destroyUniformBuffers() {
		for (size_t i = 0; i < m_uniformBuffers.size(); i++) {
			vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr); // destroy the buffer
			vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr); // free the memory
		}
	}

	void updateUniformBuffer(uint32_t currentImage, VkExtent2D swapChainExtent) {
		static auto startTime = std::chrono::high_resolution_clock::now(); // start time

		auto currentTime = std::chrono::high_resolution_clock::now(); // current time
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); // time since start
		UBO ubo = {};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.f), glm::vec3(0.0f, 0.0f, 1.0f)); 
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1; // flip the y axis because openGL standards in glm
		memcpy(m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo)); // copy the data to the buffer
	}
private:
	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	std::vector<void*> m_uniformBuffersMapped;
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;

	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UBO); // size of the uniform buffer object
		m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			BufferUtils::createBuffer(m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]); // create the uniform buffer
			vkMapMemory(m_device, m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]); // map the memory
		}
	}
};
