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
#include <stb_image.h>

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
	std::shared_ptr<DescriptorManager> m_descriptorManager; // descriptor set layout object
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
		m_uniformBuffers = std::make_shared<UniformBuffers>(m_device->getDevice(), m_device->getPhysicalDevice());
		m_descriptorManager = std::make_shared<DescriptorManager>(m_device->getDevice(), m_uniformBuffers->getUniformBuffers());
		m_pipeline = std::make_shared<Pipeline>(m_renderPass->getRenderPass(), m_device->getDevice(), m_swapChain->getSwapChainExtent(), m_swapChain->getSwapChainImageViews(), m_descriptorManager->getDescriptorSetLayout()); // create a pipeline object
		m_swapChain->createFrameBuffers(m_renderPass->getRenderPass());
		m_commandPool = std::make_shared<CommandPool>(m_device->getDevice(), m_device->getPhysicalDevice(), m_window->getSurface()); // create a command pool object
		m_triangle = std::make_shared<Mesh>(m_device->getDevice(), m_device->getPhysicalDevice(), m_commandPool->getCommandPool(), m_device->getGraphicsQueue(), m_vertices, m_indices);
		createSyncObjects();
		//createDescriptorPool();
		//createDescriptorSets();
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
		m_descriptorManager->destroyDescriptorManager();
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

		//DESCRIPTOR SETS
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getPipelineLayout(), 0, 1, &m_descriptorManager->getDescriptorSet(currentFrame), 0, nullptr); // bind the descriptor sets

		m_triangle->draw(commandBuffer); // draw the triangle

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



	void createTextureImage() {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("./assets/textures/statue.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		BufferUtils::createBuffer(m_device->getDevice(), m_device->getPhysicalDevice(), imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		
		void* data;
		vkMapMemory(m_device->getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_device->getDevice(), stagingBufferMemory);
		
		stbi_image_free(pixels);

		createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);
	}

	VkImage m_textureImage;
	VkDeviceMemory m_textureImageMemory;

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0; // for multisampling
	
		if (vkCreateImage(m_device->getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("falied to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_device->getDevice(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = BufferUtils::findMemoryType(m_device->getPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(m_device->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(m_device->getDevice(), image, imageMemory, 0);

	}
};
