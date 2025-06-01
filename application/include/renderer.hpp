#pragma once

#include <vulkan/vulkan.h>
#include "window.hpp"
#include <memory>
#include "instance.hpp"
#include "device.hpp"
#include "swapChain.hpp"
#include "renderPass.hpp"
#include "pipeline.hpp"
#include "commandPool.hpp"
#include "descriptorManager.hpp"
#include "uniformBuffers.hpp"
#include "model.hpp"

/**
 * @class Renderer
 * @brief Manages and holds all the resources for the Vulkan rendering process.
 */
class Renderer {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:
	std::shared_ptr<Window> m_window; ///< Pointer to the window object used for rendering.
	std::shared_ptr<VKInstance> m_instance; ///< Pointer to the Vulkan instance object used for managing Vulkan resources.
	std::shared_ptr<Device> m_device; ///< Pointer to the Vulkan device object used for interacting with the GPU.
	std::shared_ptr<VKSwapChain> m_swapChain; ///< Pointer to the Vulkan swapchain object used for managing image presentation.
	std::shared_ptr<RenderPass> m_renderPass; ///< Pointer to the Vulkan render pass object used for defining how rendering is done.
	std::shared_ptr<Pipeline> m_pipeline; ///< Pointer to the Vulkan pipeline object used for managing the graphics pipeline state and shaders.
	std::shared_ptr<CommandPool> m_commandPool; ///< Pointer to the Vulkan command pool object used for managing command buffers and command execution.
	std::shared_ptr<DescriptorManager> m_descriptorManager; ///< Pointer to the descriptor manager object used for managing descriptor sets and layouts for shader resources.
	std::shared_ptr<UniformBuffers> m_uniformBuffers; ///< Pointer to the uniform buffers object used for managing uniform buffer objects that store shader data.
	std::shared_ptr<Model> m_modelPBR; ///< Pointer to the model object used for loading and rendering 3D models with PBR materials.

	//synchronisation
	std::vector<VkSemaphore> imageAvailableSemaphores; ///< Semaphores used to signal when an image is available for rendering.
	std::vector<VkSemaphore> renderFinishedSemaphores; ///< Semaphores used to signal when rendering is finished and the image can be presented.
	std::vector<VkFence> inFlightFences; ///< Fences used to synchronize CPU and GPU operations, ensuring that the CPU waits for the GPU to finish rendering before proceeding.
	uint32_t currentFrame = 0; ///< Index of the current frame being rendered, used to manage synchronization and resource updates.

	bool framebufferResized = false; ///< Flag indicating whether the framebuffer has been resized, used to trigger swapchain recreation.

	/**
 * @brief Initialize the window by creating a Window object.
 */
	void initWindow();

	/**
	 * @brief Initialize Vulkan objects and setup rendering pipeline.
	 *
	 * Creates Vulkan instance, device, swapchain, render pass,
	 * uniform buffers, descriptor sets, pipeline, command pool,
	 * depth resources, framebuffers, synchronization objects,
	 * and loads a PBR model with textures.
	 */
	void initVulkan();

	/**
	 * @brief Run the main application loop.
	 *
	 * Polls window events and draws frames continuously until the window is closed.
	 * Waits for the device to finish before exiting.
	 */
	void mainLoop();
	/**
 * @brief Render a single frame.
 *
 * Waits for previous frame to finish, acquires next swapchain image,
 * updates uniform buffers, records command buffers, submits draw commands,
 * and presents the rendered image.
 * Handles window resizing and swapchain recreation.
 */
	void drawFrame();

	/**
	 * @brief Update uniform buffers for the current frame.
	 */
	void update();
	/**
 * @brief Cleanup Vulkan and window resources.
 *
 * Destroys swapchain, uniform buffers, descriptor sets, model,
 * pipeline, synchronization objects, command pool, device, and window.
 */
	void cleanup();
	/**
 * @brief Create synchronization objects (semaphores and fences) used for rendering frames.
 *
 * Creates MAX_FRAMES_IN_FLIGHT semaphores for image availability and render completion,
 * and fences to ensure CPU-GPU synchronization.
 * Throws runtime_error if creation fails.
 */
	void createSyncObjects();
	/**
 * @brief Record commands into a command buffer for rendering a frame.
 *
 * Records commands to begin render pass, bind pipeline,
 * set viewport and scissor, bind buffers and descriptor sets,
 * draw the model, and end the render pass.
 *
 * @param commandBuffer The Vulkan command buffer to record commands into.
 * @param imageIndex The index of the swapchain image to render to.
 * @throws std::runtime_error if command buffer recording fails.
 */
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	/**
 * @brief Handle window resize events.
 *
 * Waits for the window framebuffer to be non-zero size,
 * then recreates the swapchain and related resources.
 */
	void windowResize();
};
