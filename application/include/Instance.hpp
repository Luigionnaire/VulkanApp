#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include "validationLayersConfig.hpp"
#include "debugManager.hpp"
#include "extensions.hpp"

class VKInstance {

public:

	VKInstance();

	~VKInstance();

	const VkInstance& getInstance() const;
private:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
};