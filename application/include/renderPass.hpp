#pragma once
#include "vulkan/vulkan.h"
#include <vector>
#include <stdexcept>


class RenderPass {
public:
	RenderPass(VkDevice device, VkFormat swapChainImageFormat, VkFormat depthFormat);

	VkRenderPass getRenderPass() const { return m_renderPass; }
private:
	VkFormat m_swapChainImageFormat;
	VkDevice m_device;
	VkRenderPass m_renderPass;
	VkFormat m_depthFormat;

	void createRenderPass();
};
