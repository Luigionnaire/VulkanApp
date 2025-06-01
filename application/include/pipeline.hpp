#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "shaderManager.hpp"
#include "vertex.hpp"
/**
 * @class Pipeline
 * @brief Encapsulates Vulkan graphics pipeline creation and management.
 *
 * Constructs a graphics pipeline with shaders, fixed-function stages,
 * and pipeline layout using a render pass and swapchain details.
 */
class Pipeline {
public:
	/**
 * @brief Constructs a Pipeline object and creates the graphics pipeline.
 * @param renderPass The render pass this pipeline will be used with.
 * @param device The Vulkan logical device.
 * @param swapChainExtent The extent (width, height) of the swapchain images.
 * @param swapChainImageViews Image views associated with the swapchain.
 * @param descriptorSetLayout Descriptor set layout for resource binding.
 */
	Pipeline(VkRenderPass renderPass, VkDevice device, VkExtent2D swapChainExtent, std::vector<VkImageView> swapChainImageViews, VkDescriptorSetLayout descriptorSetLayout);

	/**
 * @brief Creates the Vulkan graphics pipeline, including shader stages,
 * vertex input, input assembly, viewport/scissor, rasterizer,
 * multisampling, color blending, depth/stencil testing, and pipeline layout.
 * Throws std::runtime_error on failure.
 */
	void createGraphicsPipeline();

	/**
 * @brief Destroys the graphics pipeline and its layout.
 */
	const void destroyPipeline();

	/**
 * @brief Updates the stored swap chain extent (e.g. on window resize).
 * @param extent New swap chain extent.
 */
	void getUpdatedSwapChainExtent(VkExtent2D extent);

	VkPipeline getPipeline() const { return m_pipeline; }
	VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
private:
	VkPipeline m_pipeline; ///< Vulkan graphics pipeline handle.
	VkPipelineLayout m_pipelineLayout; ///< Vulkan pipeline layout handle.
	VkRenderPass m_renderPass; ///< Render pass this pipeline is associated with.
	VkDevice m_device; ///< Vulkan logical device.
	VkExtent2D m_swapChainExtent; ///< Extent of the swapchain images.
	std::vector<VkImageView> m_swapChainImageViews; ///< Image views for the swapchain.
	VkDescriptorSetLayout m_descriptorSetLayout; ///< Descriptor set layout for resource binding.
};