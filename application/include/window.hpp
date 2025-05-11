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

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto appWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        appWindow->framebufferResized = true;
    }

private:
    GLFWwindow* m_window;
    const uint32_t width = 800;
    const uint32_t height = 600;
    bool framebufferResized;

};
