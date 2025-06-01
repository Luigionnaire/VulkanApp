#pragma once

#include <vector>

/**
 * @namespace ValidationLayersConfig
 * @brief Configuration for enabling Vulkan validation layers.
 *
 * Contains a flag to toggle validation layers based on build type and
 * a list of requested validation layers.
 */
namespace ValidationLayersConfig
{
#ifdef NDEBUG
	inline const bool enableValidationLayers = false; ///< Disable validation layers in release builds for performance.
#else
	inline const bool enableValidationLayers = true; ///< Enable validation layers in debug builds for debugging purposes.
#endif

    const std::vector<const char*> validationLayers = { 
        "VK_LAYER_KHRONOS_validation"
	}; ///< List of validation layers to enable if validation layers are requested.
};