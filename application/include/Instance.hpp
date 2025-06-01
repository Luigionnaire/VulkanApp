#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include "validationLayersConfig.hpp"
#include "debugManager.hpp"
#include "extensions.hpp"
/**
 * @class VKInstance
 * @brief Encapsulates Vulkan instance creation and destruction, including debug messenger setup.
 */
class VKInstance {

public:

	VKInstance();

	~VKInstance();

	const VkInstance& getInstance() const;
private:
	VkInstance m_instance; ///< The Vulkan instance handle.
	VkDebugUtilsMessengerEXT m_debugMessenger; ///< Debug messenger for validation layers.
};