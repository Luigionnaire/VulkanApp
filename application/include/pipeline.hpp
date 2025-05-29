#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "shaderManager.hpp"
#include "vertex.hpp"

class Pipeline {
public:
	Pipeline(VkRenderPass renderPass, VkDevice device, VkExtent2D swapChainExtent, std::vector<VkImageView> swapChainImageViews, VkDescriptorSetLayout descriptorSetLayout ) :
		m_renderPass(renderPass),
		m_device(device),
		m_swapChainExtent(swapChainExtent),
		m_swapChainImageViews(swapChainImageViews),
		m_descriptorSetLayout(descriptorSetLayout)
	{
		createGraphicsPipeline();
	}
	void createGraphicsPipeline() {
		auto vertShaderCode = ShaderManager::readFile("./assets/shaders/vert.spv");
		auto fragShaderCode = ShaderManager::readFile("./assets/shaders/frag.spv");

		VkShaderModule vertShaderModule = ShaderManager::createShaderModule(vertShaderCode, m_device);
		VkShaderModule fragShaderModule = ShaderManager::createShaderModule(fragShaderCode, m_device);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main"; // entry point

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main"; // entry point

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // triangle list
		inputAssembly.primitiveRestartEnable = VK_FALSE; // no primitive restart

		
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_swapChainExtent.width;
		viewport.height = (float)m_swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{}; // defines in which regions pixels will be stored
		scissor.offset = { 0, 0 };
		scissor.extent = m_swapChainExtent;


		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE; // discard fragments outside of the depth range
		rasterizer.rasterizerDiscardEnable = VK_FALSE; // pass the fragments to the next stage (not if VK_TRUE)
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f; // larger values need wideLines GPU feature
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // adjusted to fit the Y flip done due to openGL standards in glm
		rasterizer.depthBiasEnable = VK_FALSE; // useful for shadow mapping

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE; // multisampling
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 1 sample per pixel

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE; // no blending

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional


		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE; // enable depth testing
		depthStencil.depthWriteEnable = VK_TRUE; // enable depth writing
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // less than comparison
		depthStencil.depthBoundsTestEnable = VK_FALSE; // optional
		depthStencil.minDepthBounds = 0.0f; // optional
		depthStencil.maxDepthBounds = 1.0f; // optional
		depthStencil.stencilTestEnable = VK_FALSE; // optional
		depthStencil.front = {}; // optional
		depthStencil.back = {}; // optional

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1; // number of descriptor set layouts
		pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout; // descriptor set layout

		if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.renderPass = m_renderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // only used if VK_PIPELINE_CREATE_DERIVATIVE_BIT is set
		pipelineInfo.basePipelineIndex = -1;			  // in VkGraphicsPipelineCreateInfo

		if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) // can establish pipeline caching and create multiple pipelines at once
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		ShaderManager::destroyShaderModule(vertShaderModule, m_device);
		ShaderManager::destroyShaderModule(fragShaderModule, m_device);
	}
	
	VkPipeline getPipeline() const { return m_pipeline; }
	VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }

	const void destroyPipeline() {
		vkDestroyPipeline(m_device, m_pipeline, nullptr);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	}
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