#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <stdexcept>
#include "uniformBuffers.hpp"
/**
 * @class DescriptorManager
 * @brief Manages Vulkan descriptor sets, layouts, and descriptor pool creation for uniform buffers and sampled images.
 */
class DescriptorManager {
public:
    /**
 * @brief Constructs the DescriptorManager with the given device and uniform buffers.
 *
 * @param device The Vulkan logical device.
 * @param buffers A list of uniform buffers, one for each frame/image.
 */
    DescriptorManager(VkDevice device, std::vector<VkBuffer> buffers);
    ~DescriptorManager() = default;

    void destroyDescriptorManager();

    VkDescriptorSetLayout getDescriptorSetLayout() const;

    /**
  * @brief Gets a specific descriptor set by index.
  *
  * @param index The index of the descriptor set to retrieve.
  * @return A const reference to the VkDescriptorSet.
  */
    const VkDescriptorSet& getDescriptorSet(uint32_t index) const;

    /**
 * @brief Creates descriptor sets for the given image views and samplers.
 *
 * @param imageViews An array of 4 image views to be used in the descriptor set.
 * @param sampler An array of 4 samplers to be associated with the image views.
 */
    void createDescriptorSets(const std::array<VkImageView, 4>& imageViews,
        const std::array<VkSampler, 4>& sampler);

private:
	VkDevice m_device; 					 ///< Vulkan logical device.
	VkDescriptorPool m_descriptorPool; ///< Descriptor pool for managing descriptor sets.
	VkDescriptorSetLayout m_descriptorSetLayout; ///< Layout for the descriptor sets.
	std::vector<VkDescriptorSet> m_descriptorSets; ///< Vector of descriptor sets, one for each frame/image.
	std::vector<VkBuffer> m_uniformBuffers; ///< List of uniform buffers, one for each frame/image.

    /**
    * @brief Creates the descriptor set layout used for all descriptor sets.
    */
    void createDescriptorSetLayout();

    /**
    * @brief Creates the Vulkan descriptor pool used to allocate descriptor sets.
    */
    void createDescriptorPool();
};
