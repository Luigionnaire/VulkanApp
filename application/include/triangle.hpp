#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> // GLFW header
#include <vulkan/vulkan.h>

class Triangle {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	uint32_t glfwExtensionCount = 0; // number of extensions
	const char** glfwExtensions; // array of extensions

	GLFWwindow* window;
	VkInstance instance;
	void createInstance() {
		VkApplicationInfo appInfo{}; // optional struct to allow Vulkan to optimise for this case
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{}; // non-optional struct
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // get the required extensions

		createInfo.enabledExtensionCount = glfwExtensionCount; // look into
		createInfo.ppEnabledExtensionNames = glfwExtensions; // look into
		createInfo.enabledLayerCount = 0; // for later

		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance); // create the instance

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) { // check for errors
			throw std::runtime_error("failed to create instance!"); // throw an error
		}
	}

	void initWindow() {
		//Initialise GLFW
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  //create a window without OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disable resizing
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan App", nullptr, nullptr);
	}
	void initVulkan() {
		// Initialize Vulkan
		createInstance();
	}
	void mainLoop() {
		// Main loop
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents(); // Poll for events
		}	
	}
	void cleanup() {
		// Cleanup Vulkan
		glfwDestroyWindow(window); // Destroy window
		glfwTerminate(); // Terminate GLFW
	}
};