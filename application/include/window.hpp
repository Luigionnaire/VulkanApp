#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

class Window {
public:
    Window();

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
    void createSurface(const VkInstance& instance);

    void destroySurface(const VkInstance& instance);
private:
    GLFWwindow* m_window;
	VkSurfaceKHR m_surface = nullptr;
    const uint32_t width = 800;
    const uint32_t height = 600;
    bool framebufferResized = false;

};
