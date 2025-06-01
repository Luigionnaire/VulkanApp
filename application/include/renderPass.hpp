#pragma once
#include "vulkan/vulkan.h"
#include <vector>
#include <stdexcept>

/**
 * @class RenderPass
 * @brief Manages Vulkan render pass creation.
 */
class RenderPass {
public:
	/**
  * @brief Construct a new RenderPass object.
  *
  * @param device Vulkan logical device handle.
  * @param swapChainImageFormat Format of the swap chain images.
  * @param depthFormat Format of the depth buffer image.
  */
	RenderPass(VkDevice device, VkFormat swapChainImageFormat, VkFormat depthFormat);

	VkRenderPass getRenderPass() const { return m_renderPass; }
private:
	VkFormat m_swapChainImageFormat; ///< Format of the swap chain images.
	VkDevice m_device; ///< Vulkan logical device handle.
	VkRenderPass m_renderPass; ///< Vulkan render pass handle.
	VkFormat m_depthFormat; ///< Format of the depth buffer image.

	/**
* @brief Creates the Vulkan render pass with color and depth attachments.
*
* This function sets up the color and depth attachments, subpass, and
* subpass dependency, then creates the render pass object.
*
* @throws std::runtime_error if render pass creation fails.
*/
	void createRenderPass();
};
