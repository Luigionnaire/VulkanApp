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
const int MAX_FRAMES_IN_FLIGHT = 2; // frames processed concurrently

const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; // swapchain extension


struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily; // for the window surface
	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value(); // std::optional functionality
	}
};

struct SwapChainSupportDetails {  
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

// vertices
struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex); 
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // move to next data entry after each vertex
		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{}; // two because we are passing a vec2 and a vec3
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0; 
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // 2 floats
		attributeDescriptions[0].offset = offsetof(Vertex, pos); // offset of the position data

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // 3 floats
		attributeDescriptions[1].offset = offsetof(Vertex, color); // offset of the color data

		return attributeDescriptions;
	}
};

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

	//VkQueue presentQueue;

	//swapchain
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews; 
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent; 


	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline; 

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	// devices
	//VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; 
	//VkDevice device; 
	//VkQueue graphicsQueue;

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
		/*pickPhysicalDevice();
		createLogicalDevice();*/
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFrameBuffers();
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

		VkResult result = vkAcquireNextImageKHR(m_device->getDevice(), swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex); // acquire the next image from the swapchain
		if (result == VK_ERROR_OUT_OF_DATE_KHR) { // check if the swapchain is out of date
			recreateSwapChain(); // recreate the swapchain
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { // check for errors
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		vkResetFences(m_device->getDevice(), 1, &inFlightFences[currentFrame]); // only reset fence if we are submitting work

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
		if (vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) { // submit the command buffer
			throw std::runtime_error("failed to submit draw command buffer!");
		}
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // allow to choose between multiple swapchains
		result = vkQueuePresentKHR(m_device->getPresentQueue(), &presentInfo); // present the image

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
		cleanupSwapChain();

		vkDestroyBuffer(m_device->getDevice(), indexBuffer, nullptr);
		vkFreeMemory(m_device->getDevice(), indexBufferMemory, nullptr);

		vkDestroyBuffer(m_device->getDevice(), vertexBuffer, nullptr); 
		vkFreeMemory(m_device->getDevice(), vertexBufferMemory, nullptr);

		vkDestroyPipeline(m_device->getDevice(), graphicsPipeline, nullptr); 
		vkDestroyPipelineLayout(m_device->getDevice(), pipelineLayout, nullptr); 
		vkDestroyRenderPass(m_device->getDevice(), renderPass, nullptr);

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
	void cleanupSwapChain() {
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(m_device->getDevice(), swapChainFramebuffers[i], nullptr); 
		}
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			vkDestroyImageView(m_device->getDevice(), swapChainImageViews[i], nullptr); 
		}
		vkDestroySwapchainKHR(m_device->getDevice(), swapChain, nullptr); 
	}


	//void pickPhysicalDevice() {

	//	uint32_t deviceCount = 0;
	//	vkEnumeratePhysicalDevices(m_instance->getInstance(), &deviceCount, nullptr); // get the number of devices
	//	if (deviceCount == 0) {
	//		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	//	}
	//	std::vector<VkPhysicalDevice> devices(deviceCount);
	//	vkEnumeratePhysicalDevices(m_instance->getInstance(), &deviceCount, devices.data());
	//	for (const auto& device : devices) {
	//		if (isDeviceSuitable(device)) {
	//			m_device->getPhysicalDevice() = device;
	//			break;
	//		}
	//	}
	//	if (physicalDevice == VK_NULL_HANDLE) {
	//		throw std::runtime_error("failed to find a suitable GPU!");
	//	}
	//}
	//void createLogicalDevice() {
	//	QueueFamilyIndices indices = findQueueFamilies(m_device->getPhysicalDevice());

	//	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	//	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() }; // set of unique queue families
	//	float queuePriority = 1.0f;

	//	for (uint32_t queueFamily : uniqueQueueFamilies) { // loop through the queue families
	//		VkDeviceQueueCreateInfo queueCreateInfo{};
	//		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	//		queueCreateInfo.queueFamilyIndex = queueFamily; // set the family index
	//		queueCreateInfo.queueCount = 1; // set the queue count
	//		queueCreateInfo.pQueuePriorities = &queuePriority; // set the priority
	//		queueCreateInfos.push_back(queueCreateInfo); // add to the vector
	//	}

	//	VkPhysicalDeviceFeatures deviceFeatures{};

	//	VkDeviceCreateInfo createInfo{};

	//	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	//	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	//	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	//	createInfo.pEnabledFeatures = &deviceFeatures;
	//	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	//	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	//	if (ValidationLayersConfig::enableValidationLayers) {
	//		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayersConfig::validationLayers.size());
	//		createInfo.ppEnabledLayerNames = ValidationLayersConfig::validationLayers.data();
	//	}
	//	else {
	//		createInfo.enabledLayerCount = 0;
	//	}

	//	if (vkCreateDevice(m_device->getPhysicalDevice(), &createInfo, nullptr, &device) != VK_SUCCESS) { 
	//		throw std::runtime_error("failed to create logical device!"); 
	//	}

	//	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue); // implicitly destroyed
	//	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue); // implicitly destroyed
	//}

	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_device->getPhysicalDevice());

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_window->getSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; 

		QueueFamilyIndices indices = findQueueFamilies(m_device->getPhysicalDevice());
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
		if (indices.graphicsFamily != indices.presentFamily) { // doing this for ownership reasons (lookup)
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // concurrent mode
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices; // set the indices
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // exclusive mode
			createInfo.queueFamilyIndexCount = 0; // optional
			createInfo.pQueueFamilyIndices = nullptr; // optional
		}
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform; 
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; 
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; // ignore pixels that are obscured(like if behind another window)
		createInfo.oldSwapchain = VK_NULL_HANDLE; // resizing stuff, look up later

		if (vkCreateSwapchainKHR(m_device->getDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) { 
			throw std::runtime_error("failed to create swap chain!"); 
		}
		vkGetSwapchainImagesKHR(m_device->getDevice(), swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount); 
		vkGetSwapchainImagesKHR(m_device->getDevice(), swapChain, &imageCount, swapChainImages.data());

		// save these to member variables for later
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}
	void createImageViews(){
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_device->getDevice(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create image views!");
			}
		}
	}
	void createGraphicsPipeline() {
		auto vertShaderCode = readFile("./assets/shaders/vert.spv");
		auto fragShaderCode = readFile("./assets/shaders/frag.spv");

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

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
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{}; // defines in which regions pixels will be stored
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;


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
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		if (vkCreatePipelineLayout(m_device->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
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
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass; 
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // only used if VK_PIPELINE_CREATE_DERIVATIVE_BIT is set
		pipelineInfo.basePipelineIndex = -1;			  // in VkGraphicsPipelineCreateInfo

		if (vkCreateGraphicsPipelines(m_device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) // can establish pipeline caching and create multiple pipelines at once
		{
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		vkDestroyShaderModule(m_device->getDevice(), fragShaderModule, nullptr);
		vkDestroyShaderModule(m_device->getDevice(), vertShaderModule, nullptr);
	}
	void createRenderPass() {
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
		
		if (vkCreateRenderPass(m_device->getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!"); 
		}

	}
	void createFrameBuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_device->getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}
	void createCommandPool(){
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_device->getPhysicalDevice());

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

		cleanupSwapChain();
		createSwapChain();
		createImageViews();
		createFrameBuffers();
	}
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // optional
		beginInfo.pInheritanceInfo = nullptr; // optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		//RENDER PASS
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;
		VkClearValue clearColor = { 0.012f, 0.018f, 0.02f, 1.0f }; // clear color
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor; // clear color

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // begin render pass

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline); // bind the pipeline

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

	bool isDeviceSuitable(VkPhysicalDevice device) // can be used to only allow certain devices based on capabilities
	{ 
		QueueFamilyIndices indices = findQueueFamilies(device);
		bool extensionsSupported = checkDeviceExtensionSupport(device);
		bool swapChainAdequate = false;

		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device); // get the swap chain support
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty(); // check if the swap chain is adequate
		}
		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}


	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end()); 

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName); 
		}
		return requiredExtensions.empty(); // if the set is empty, all required extensions are supported
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) { // callback for window resize
		auto app = reinterpret_cast<Triangle*>(glfwGetWindowUserPointer(window)); // get the app pointer
		app->framebufferResized = true; // set the resized flag
	}

	
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary); // open the file in binary mode at end of file

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!"); 
		}
		size_t fileSize = (size_t)file.tellg(); // get the size of the file
		std::vector<char> buffer(fileSize); // create a buffer of the size of the file

		file.seekg(0); // go back to the beginning of the file
		file.read(buffer.data(), fileSize); // read the file into the buffer

		file.close();
		return buffer;
	}
	
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); // get the number of queue families
		
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data()); // get the queue families
		
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, 0, m_window->getSurface(), &presentSupport); // check if the device supports the surface
		// loop through the queue families
		int i = 0;
		for (const auto& queueFamily : queueFamilies) { // loop through the queue families
			if (presentSupport) { 
				indices.presentFamily = i; // set the present family
			}
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { // check if the family supports graphics
				indices.graphicsFamily = i; // set the graphics family
			}
			if (indices.isComplete()) {
				break; 
			}
			i++;
		}
		return indices;
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_window->getSurface(), &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_window->getSurface(), &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_window->getSurface(), &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_window->getSurface(), &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_window->getSurface(), &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { // check for the format
				return availableFormat; // return the format
			}
		}
		return availableFormats[0]; // return the first format if not found
	}	
	
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { // check for mailbox mode
				return availablePresentMode; // return the mode
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR; // return FIFO mode if not found
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(m_window->getWindow(), &width, &height);
			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};
			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
			return actualExtent;
		}
	}

	VkShaderModule createShaderModule(const std::vector<char>& code) {
   
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_device->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
		return shaderModule;
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
