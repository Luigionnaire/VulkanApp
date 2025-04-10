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
	GLFWwindow* window;
	void initWindow() {
		//Initialise GLFW
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  //create a window without OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disable resizing
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan App", nullptr, nullptr);
	}
	void initVulkan() {
		// Initialize Vulkan
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