#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>

class Window {
public:
    Window() {
        //Initialise GLFW
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  //create a window without OpenGL context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // disable resizing
        m_window = glfwCreateWindow(width, height, "Class window", nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    }

	GLFWwindow* getWindow() {
		return m_window;
	}

	VkSurfaceKHR& getSurface() {
		return m_surface;
	}

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto appWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        appWindow->framebufferResized = true;
    }

    void createSurface(const VkInstance& instance) { //create window
        if (glfwCreateWindowSurface(instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!"); // throw an error
        }
    }

    void destroySurface(const VkInstance& instance)
    {
        vkDestroySurfaceKHR(instance, m_surface, nullptr);
    }

private:
    GLFWwindow* m_window;
	VkSurfaceKHR m_surface;
    const uint32_t width = 800;
    const uint32_t height = 600;
    bool framebufferResized = false;

};
