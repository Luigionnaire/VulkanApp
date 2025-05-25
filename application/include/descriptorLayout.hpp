#pragma once

#include <vulkan/vulkan.h>
#include "uniformBuffers.hpp"

class DescriptorManager {
public:
	// Constructor
	DescriptorManager(VkDevice device, std::vector<VkBuffer> buffers) : m_device(device), m_uniformBuffers(buffers) {
		createDescriptorSetLayout();
		createDescriptorPool();
		createDescriptorSets();
	}

	// Destructor
	void destroyDescriptorManager() {
		vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr); // destroy descriptor pool

	}

	VkDescriptorSetLayout getDescriptorSetLayout() const {
		return m_descriptorSetLayout;
	}

	const VkDescriptorSet& getDescriptorSet(uint32_t index) const {
		if (index < m_descriptorSets.size()) {
			return m_descriptorSets[index];
		}
		throw std::out_of_range("Index out of range for descriptor sets");
	}
private:
	VkDevice m_device; // logical device object
	VkDescriptorPool m_descriptorPool; // descriptor pool for uniform buffers
	VkDescriptorSetLayout m_descriptorSetLayout; // descriptor set layout object
	std::vector<VkDescriptorSet> m_descriptorSets;
	std::vector<VkBuffer> m_uniformBuffers; // uniform buffer for the descriptor set

	void createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // optional

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;
		if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}


	}

	void createDescriptorPool() {
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}

	}

	void createDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UBO);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // for image data
			descriptorWrite.pTexelBufferView = nullptr; // for  buffer views
			vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
		}
	}
};