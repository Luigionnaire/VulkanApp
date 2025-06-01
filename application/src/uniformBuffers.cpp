#pragma once

#include "uniformBuffers.hpp"

UniformBuffers::UniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice) : m_device(device), m_physicalDevice(physicalDevice) {
	createUniformBuffers();
	ubo = {};

}
void UniformBuffers::destroyUniformBuffers() {
	for (size_t i = 0; i < m_uniformBuffers.size(); i++) {
		vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr); // destroy the buffer
		vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr); // free the memory
	}
}

void UniformBuffers::updateUniformBuffer(uint32_t currentImage, VkExtent2D swapChainExtent) {
	static auto startTime = std::chrono::high_resolution_clock::now(); // start time

	auto currentTime = std::chrono::high_resolution_clock::now(); // current time
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); // time since start
	ubo.model = glm::mat4(1.0f); // initialize model matrix to identity
	ubo.model = glm::rotate(ubo.model, time * glm::radians(20.f), glm::vec3(1.0f, 1.0f, 1.0f)); // rotate the model based on timeu		
	ubo.view = glm::lookAt(glm::vec3(0.0f, 1.0f, -3.f), glm::vec3(0.0f, -2.0f, 10.0f), glm::vec3(0.0f, 1.f, 0.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1; // flip the y axis because openGL standards in glm
	memcpy(m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo)); // copy the data to the buffer
}

VkBuffer UniformBuffers::getUniformBuffer(uint32_t index) const {
	if (index < m_uniformBuffers.size()) {
		return m_uniformBuffers[index];
	}
	throw std::out_of_range("Index out of range for uniform buffers");
}

void UniformBuffers::createUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(UBO); // size of the uniform buffer object
	m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		BufferUtils::createBuffer(m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]); // create the uniform buffer
		vkMapMemory(m_device, m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]); // map the memory
	}
}