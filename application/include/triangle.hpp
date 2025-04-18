#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> // GLFW header
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
const bool enableValidationLayers = false; // disable validation layers in release mode
#else
const bool enableValidationLayers = true; // enable validation layers in debug mode
#endif

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	bool isComplete() {
		return graphicsFamily.has_value(); // std::optional functionality
	}
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

class Triangle {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:

	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;


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
		setupDebugMessenger();
		pickPhysicalDevice();
	}
	void mainLoop() {
		// Main loop
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents(); // Poll for events
		}	
	}
	void cleanup() {
		// Cleanup Vulkan
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window); // Destroy window
		glfwTerminate(); // Terminate GLFW
	}
	void createInstance() {

		if (enableValidationLayers && !checkValidationLayerSupport()) { // check for validation layers
			throw std::runtime_error("validation layers requested, but not available!"); // throw an error
		}
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

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size()); // look into
		createInfo.ppEnabledExtensionNames = extensions.data(); // look into

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0; // no layers
			createInfo.pNext = nullptr; // no next struct
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) { // check for errors
			throw std::runtime_error("failed to create instance!"); // throw an error
		}
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) 
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}
	void setupDebugMessenger() {
		if (!enableValidationLayers) return;
		
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo); // populate the create info struct
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) 
		{ 
			throw std::runtime_error("failed to set up debug messenger"); // throw an error
		}

	}
	std::vector<const char*> getRequiredExtensions() { // debug messenger extension
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount); 

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // add debug utils extension
		}
		return extensions;
	}
	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr); // get the number of layers
		std::vector<VkLayerProperties> availableLayers(layerCount); // create a vector of layers
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()); // get the layers
		
		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				return false;
			}
		}
		return true;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,VkDebugUtilsMessageTypeFlagsEXT messageType,const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,void* pUserData) 
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}


	void pickPhysicalDevice() {
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // create a physical device handle

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr); // get the number of devices
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}
		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	bool isDeviceSuitable(VkPhysicalDevice device) // can be used to only allow certain devices based on capabilities
	{ 
		QueueFamilyIndices indices = findQueueFamilies(device);
		return indices.isComplete(); // std::optional functionality
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); // get the number of queue families
		
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data()); // get the queue families
		
		int i = 0;
		for (const auto& queueFamily : queueFamilies) { // loop through the queue families
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
};