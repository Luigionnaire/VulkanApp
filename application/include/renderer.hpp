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
#include "descriptorManager.hpp"
#include "uniformBuffers.hpp"
#include "texture.hpp"
#include "model.hpp"

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

	std::shared_ptr<Model> m_modelPBR;

	//synchronization
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	bool framebufferResized = false;

	uint32_t currentFrame = 0;

	void initWindow();

	void initVulkan();

	void mainLoop();

	void drawFrame();

	void update();

	void cleanup();

	void createSyncObjects();

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	void windowResize();
};
