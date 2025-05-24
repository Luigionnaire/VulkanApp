#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> // GLFW header
#include <vulkan/vulkan.h>
#include <algorithm>
#include <glm/glm.hpp>
#include <array>
#include "window.hpp"
#include <memory>
#include "debugManager.hpp"
#include "extensions.hpp"
#include "instance.hpp"
#include "device.hpp"
#include "swapChain.hpp"
#include "renderPass.hpp"
#include "pipeline.hpp"
#include "commandPool.hpp"
#include "mesh.hpp"
#include "descriptorLayout.hpp"
#include "uniformBuffers.hpp"



class Renderer {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:
	std::shared_ptr<Window> m_window; // window object and surface
	std::shared_ptr<VKInstance> m_instance; 
	std::shared_ptr<Device> m_device; // logical and physical device object
	std::shared_ptr<VKSwapChain> m_swapChain; 
	std::shared_ptr<RenderPass> m_renderPass; // render pass object
	std::shared_ptr<Pipeline> m_pipeline; // pipeline object
	std::shared_ptr<CommandPool> m_commandPool; // command pool object
	std::shared_ptr<DescriptorLayout> m_descriptorSetLayout; // descriptor set layout object
	std::shared_ptr<UniformBuffers> m_uniformBuffers; // uniform buffer object

	const std::vector<Vertex> m_vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}}
	};

	const std::vector<uint16_t> m_indices = {
		0, 1, 2,
		2, 3, 0
	};

	std::shared_ptr<Mesh> m_triangle;
	//synchronization
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	 
	bool framebufferResized = false;

	uint32_t currentFrame = 0;

	void initWindow() {
		m_window = std::make_shared<Window>(); // create a window object
	}
	void initVulkan()  {
		m_instance = std::make_shared<VKInstance>(); // create a instance object
		m_window->createSurface(m_instance->getInstance()); // window
		m_device = std::make_shared<Device>(m_instance->getInstance(), m_window->getSurface()); // create a device object
		m_swapChain = std::make_shared<VKSwapChain>(m_window->getSurface(), m_device->getPhysicalDevice(), m_device->getDevice(), m_window->getWindow());
		m_renderPass = std::make_shared<RenderPass>(m_device->getDevice(), m_swapChain->getSwapChainImageFormat()); // create a render pass object
		m_descriptorSetLayout = std::make_shared<DescriptorLayout>(m_device->getDevice());
		m_pipeline = std::make_shared<Pipeline>(m_renderPass->getRenderPass(), m_device->getDevice(), m_swapChain->getSwapChainExtent(), m_swapChain->getSwapChainImageViews()); // create a pipeline object
		m_swapChain->createFrameBuffers(m_renderPass->getRenderPass());
		m_commandPool = std::make_shared<CommandPool>(m_device->getDevice(), m_device->getPhysicalDevice(), m_window->getSurface()); // create a command pool object
		m_triangle = std::make_shared<Mesh>(m_device->getDevice(), m_device->getPhysicalDevice(), m_commandPool->getCommandPool(), m_device->getGraphicsQueue(), m_vertices, m_indices);
		m_uniformBuffers = std::make_shared<UniformBuffers>(m_device->getDevice(), m_device->getPhysicalDevice());
		createSyncObjects();
	}
	void mainLoop() {
		// Main loop
		while (!glfwWindowShouldClose(m_window->getWindow())) {
			glfwPollEvents(); // Poll for events
			drawFrame();
		}	

		vkDeviceWaitIdle(m_device->getDevice()); // wait for the device to finish
	}
	
	void drawFrame() {
		vkWaitForFences(m_device->getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX); // wait for previous frame
		
		uint32_t imageIndex;

		VkResult result = vkAcquireNextImageKHR(m_device->getDevice(), m_swapChain->getSwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex); // acquire the next image from the swapchain  !!memory access!! 
		if (result == VK_ERROR_OUT_OF_DATE_KHR) { // check if the swapchain is out of date
			recreateSwapChain(); // recreate the swapchain
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { // check for errors
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		m_uniformBuffers->updateUniformBuffer(currentFrame, m_swapChain->getSwapChainExtent()); // update the uniforms

		vkResetFences(m_device->getDevice(), 1, &inFlightFences[currentFrame]); // only reset fence if we are submitting work !!memory access!!

		vkResetCommandBuffer(m_commandPool->getCommandBuffer(currentFrame), 0);
		recordCommandBuffer(m_commandPool->getCommandBuffer(currentFrame), imageIndex);

		VkSubmitInfo submitInfo{}; 
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]}; // wait for the image to be available
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // wait for the color attachment output stage
		submitInfo.waitSemaphoreCount = 1; // number of semaphores to wait for
		submitInfo.pWaitSemaphores = waitSemaphores; // semaphores to wait for
		submitInfo.pWaitDstStageMask = waitStages; // stages to wait for
		submitInfo.commandBufferCount = 1; 
		submitInfo.pCommandBuffers = &m_commandPool->getCommandBuffer(currentFrame);

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]}; // signal the render finished semaphore
		submitInfo.signalSemaphoreCount = 1; // number of semaphores to signal
		submitInfo.pSignalSemaphores = signalSemaphores; // semaphores to signal
		if (vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) { // submit the command buffer  !!memory access!!
			throw std::runtime_error("failed to submit draw command buffer!");
		}
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_swapChain->getSwapChain()};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // allow to choose between multiple swapchains
		result = vkQueuePresentKHR(m_device->getPresentQueue(), &presentInfo); // present the image  !!memory access!!

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) { 
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}


		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT; // increment the current frame
	}
	
	// cleanup functions
	void cleanup() {
		m_swapChain->cleanupSwapChain();
		m_uniformBuffers->destroyUniformBuffers();
		m_descriptorSetLayout->destroyDescriptorSetLayout();
		m_triangle->freeMemory();
		m_pipeline->destroyPipeline();
		vkDestroyRenderPass(m_device->getDevice(), m_renderPass->getRenderPass(), nullptr);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) // cleanup semaphores and fences
		{
			vkDestroySemaphore(m_device->getDevice(), renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device->getDevice(), imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device->getDevice(), inFlightFences[i], nullptr);
		}
		vkDestroyCommandPool(m_device->getDevice(), m_commandPool->getCommandPool(), nullptr);
		vkDestroyDevice(m_device->getDevice(), nullptr);
		m_window->destroySurface(m_instance->getInstance());
		glfwDestroyWindow(m_window->getWindow()); // Destroy window
		glfwTerminate(); // Terminate GLFW
	}

	void createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // signaled so it doesnt hang at the first frame
		
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(m_device->getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS 
			 || vkCreateSemaphore(m_device->getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS 
			 || vkCreateFence(m_device->getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) // create semaphores and fence
			{
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // optional
		beginInfo.pInheritanceInfo = nullptr; // optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkExtent2D swapChainExtent = m_swapChain->getSwapChainExtent();
		//RENDER PASS
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderPass->getRenderPass();
		renderPassInfo.framebuffer = m_swapChain->getSwapChainFramebuffers()[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;
		VkClearValue clearColor = { 0.012f, 0.018f, 0.02f, 1.0f }; // clear color
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor; // clear color

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // begin render pass

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getPipeline()); // bind the pipeline

		//VIEWPORT AND SCISSOR
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.height = static_cast<float>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport); // set the viewport

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor); // set the scissor

		////BUFFERS
		m_triangle->bindBuffers(commandBuffer); // bind the buffers
		m_triangle->draw(commandBuffer); // draw the triangle
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0); // draw the triangle

		vkCmdEndRenderPass(commandBuffer); // end render pass
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	// window resize
	void recreateSwapChain() {
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_window->getWindow(), &width, &height); // get the window size
		while (width == 0 || height == 0) { // wait for the window to be resized
			glfwGetFramebufferSize(m_window->getWindow(), &width, &height);
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(m_device->getDevice());

		m_swapChain->cleanupSwapChain();
		m_swapChain->createSwapChain(m_window->getSurface(), m_device->getPhysicalDevice(),m_window->getWindow());
		m_swapChain->createImageViews();
		m_swapChain->createFrameBuffers(m_renderPass->getRenderPass());
	}

	
};
