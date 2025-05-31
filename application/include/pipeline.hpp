#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "shaderManager.hpp"
#include "vertex.hpp"

class Pipeline {
public:
	Pipeline(VkRenderPass renderPass, VkDevice device, VkExtent2D swapChainExtent, std::vector<VkImageView> swapChainImageViews, VkDescriptorSetLayout descriptorSetLayout);

	void createGraphicsPipeline();
	const void destroyPipeline();

	void getUpdatedSwapChainExtent(VkExtent2D extent);

	VkPipeline getPipeline() const { return m_pipeline; }
	VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
private:
	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;
	VkRenderPass m_renderPass;
	VkDevice m_device;
	VkExtent2D m_swapChainExtent;
	std::vector<VkImageView> m_swapChainImageViews;
	VkDescriptorSetLayout m_descriptorSetLayout;
	//add shaders
};