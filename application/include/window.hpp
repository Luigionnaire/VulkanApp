#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

/**
 * @class Window
 * @brief Manages GLFW window creation and Vulkan surface integration.
 */
class Window {
public:
    /**
     * @brief Constructs and initializes a GLFW window.
     */
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

    /**
 * @brief Creates a Vulkan surface for the GLFW window.
 *
 * @param instance The Vulkan instance used to create the surface.
 * @throws std::runtime_error if surface creation fails.
 */
    void createSurface(const VkInstance& instance);

    /**
 * @brief Destroys the Vulkan surface.
 *
 * @param instance The Vulkan instance used to destroy the surface.
 */
    void destroySurface(const VkInstance& instance);
private:
    GLFWwindow* m_window;
	VkSurfaceKHR m_surface = nullptr;
    const uint32_t width = 800;
    const uint32_t height = 600;
    bool framebufferResized = false;

};
