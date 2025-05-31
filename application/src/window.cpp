#pragma once
#define GLFW_INCLUDE_VULKAN // include Vulkan headers in GLFW

#include "window.hpp"
#include <stdexcept>


Window::Window() {
    //Initialise GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  //create a window without OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // disable resizing
    m_window = glfwCreateWindow(width, height, "Vulkan App", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}



void Window::createSurface(const VkInstance& instance) { //create window
    if (glfwCreateWindowSurface(instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!"); // throw an error
    }
}

void Window::destroySurface(const VkInstance& instance)
{
    vkDestroySurfaceKHR(instance, m_surface, nullptr);
}