#pragma once

#include <vulkan/vulkan.h>

class DescriptorLayout {
public:
	// Constructor
	DescriptorLayout(VkDevice device) : m_device(device) {
		createDescriptorSetLayout();
	}

	// Destructor
	void destroyDescriptorSetLayout() {
		vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
	}

	VkDescriptorSetLayout getDescriptorSetLayout() const {
		return m_descriptorSetLayout;
	}

private:
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
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1; // number of descriptor set layouts
		pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout; // descriptor set layout


	}

	VkDescriptorSetLayout m_descriptorSetLayout; // descriptor set layout object
	VkDevice m_device; // logical device object
};