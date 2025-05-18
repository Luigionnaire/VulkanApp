#pragma once
#include "vulkan/vulkan.h"
#include <vector>
#include <stdexcept>


class RenderPass {
public:
	RenderPass(VkDevice device, VkFormat swapChainImageFormat) : m_device(device), m_swapChainImageFormat(swapChainImageFormat) {
		createRenderPass();
	}
	~RenderPass() {
		//vkDestroyRenderPass(m_device, m_renderPass, nullptr);
	}
	VkRenderPass getRenderPass() const { return m_renderPass; }
private:
	VkFormat m_swapChainImageFormat;
	VkDevice m_device;
	VkRenderPass m_renderPass;

	void createRenderPass() {
		VkFormat swapChainImageFormat = m_swapChainImageFormat;
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // 1 sample per pixel
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear the attachment
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store the attachment

		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // no stencil
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // no stencil

		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //layout before the render pass (irrelevant)
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // ready the image for presentation

		//subpass
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // specify type of subpass
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}

	}
};
