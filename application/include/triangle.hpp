#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> // GLFW header
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>
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
#include "shaderManager.hpp"
#include "pipeline.hpp"

const int MAX_FRAMES_IN_FLIGHT = 2; // frames processed concurrently

const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; // swapchain extension

// vertices


const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2,
	2, 3, 0
};



class Triangle {
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
	std::shared_ptr<Device> m_device;
	std::shared_ptr<VKSwapChain> m_swapChain; 
	std::shared_ptr<RenderPass> m_renderPass; // render pass object
	std::shared_ptr<Pipeline> m_pipeline; // pipeline object

	//std::vector<VkFramebuffer> swapChainFramebuffers;
 
	//VkRenderPass renderPass;
	//VkPipelineLayout pipelineLayout;
	//VkPipeline graphicsPipeline; 

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	//synchronization
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

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
		m_pipeline = std::make_shared<Pipeline>(m_renderPass->getRenderPass(), m_device->getDevice(), m_swapChain->getSwapChainExtent(), m_swapChain->getSwapChainImageViews()); // create a pipeline object
		m_swapChain->createFrameBuffers(m_renderPass->getRenderPass());
		createCommandPool();
		createVertexBuffer();
		createIndexBuffer();
		createCommandBuffers();
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
		vkResetFences(m_device->getDevice(), 1, &inFlightFences[currentFrame]); // only reset fence if we are submitting work !!memory access!!

		vkResetCommandBuffer(commandBuffers[currentFrame], 0);
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

		VkSubmitInfo submitInfo{}; 
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]}; // wait for the image to be available
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // wait for the color attachment output stage
		submitInfo.waitSemaphoreCount = 1; // number of semaphores to wait for
		submitInfo.pWaitSemaphores = waitSemaphores; // semaphores to wait for
		submitInfo.pWaitDstStageMask = waitStages; // stages to wait for
		submitInfo.commandBufferCount = 1; 
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame]; 

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

		vkDestroyBuffer(m_device->getDevice(), indexBuffer, nullptr);
		vkFreeMemory(m_device->getDevice(), indexBufferMemory, nullptr);

		vkDestroyBuffer(m_device->getDevice(), vertexBuffer, nullptr); 
		vkFreeMemory(m_device->getDevice(), vertexBufferMemory, nullptr);

		/*vkDestroyPipeline(m_device->getDevice(), m_pipeline->getGraphicsPipeline(), nullptr);
		vkDestroyPipelineLayout(m_device->getDevice(), pipelineLayout, nullptr); */
		m_pipeline->destroyPipeline();
		vkDestroyRenderPass(m_device->getDevice(), m_renderPass->getRenderPass(), nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) // cleanup semaphores and fences
		{
			vkDestroySemaphore(m_device->getDevice(), renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device->getDevice(), imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device->getDevice(), inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(m_device->getDevice(), commandPool, nullptr); 
		vkDestroyDevice(m_device->getDevice(), nullptr);

		m_window->destroySurface(m_instance->getInstance());

		glfwDestroyWindow(m_window->getWindow()); // Destroy window
		glfwTerminate(); // Terminate GLFW
	}
	/*void cleanupSwapChain() {
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(m_device->getDevice(), swapChainFramebuffers[i], nullptr); 
		}
		for (size_t i = 0; i < m_swapChain->getSwapChainImageViews().size(); i++) {
			vkDestroyImageView(m_device->getDevice(), m_swapChain->getSwapChainImageViews()[i], nullptr);
		}
		vkDestroySwapchainKHR(m_device->getDevice(), m_swapChain->getSwapChain(), nullptr);
	}*/

	/*void createFrameBuffers() {
		std::vector<VkImageView> swapChainImageViews = m_swapChain->getSwapChainImageViews();
		VkExtent2D swapChainExtent = m_swapChain->getSwapChainExtent();
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_renderPass->getRenderPass();
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_device->getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}*/
	void createCommandPool(){
		QueueFamily::QueueFamilyIndices queueFamilyIndices = QueueFamily::findQueueFamilies(m_device->getPhysicalDevice(), m_window->getSurface());

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // allows individual command reset
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(m_device->getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}
	void createCommandBuffers() {
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // choose between primary and secondary
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(m_device->getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer!");
		}
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

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage; // specify purpose
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // only used from graphics queue

		if (vkCreateBuffer(m_device->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_device->getDevice(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size; // size of the buffer
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(m_device->getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(m_device->getDevice(), buffer, bufferMemory, 0);  // last parameter is offset

	}
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

		//BUFFERS
		VkBuffer vertexBuffers[] = { vertexBuffer }; 
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		
		vkCmdEndRenderPass(commandBuffer); // end render pass
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_device->getDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; 

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.size = size; 
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_device->getGraphicsQueue()); // wait for the queue to finish

		vkFreeCommandBuffers(m_device->getDevice(), commandPool, 1, &commandBuffer); // cleanup command buffer

	}

	// geometry
	void createVertexBuffer() {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_device->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_device->getDevice(), stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(m_device->getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_device->getDevice(), stagingBufferMemory, nullptr);

	}
	void createIndexBuffer() {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_device->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_device->getDevice(), stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(m_device->getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_device->getDevice(), stagingBufferMemory, nullptr);
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) { // callback for window resize
		auto app = reinterpret_cast<Triangle*>(glfwGetWindowUserPointer(window)); // get the app pointer
		app->framebufferResized = true; // set the resized flag
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_device->getPhysicalDevice(), &memProperties); // get the memory properties

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) { // loop through the memory types
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) { // check if the type is supported
				return i; // return the index
			}
		}
		throw std::runtime_error("failed to find suitable memory type!"); // throw an error
	}

};
