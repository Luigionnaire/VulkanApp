#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <stdexcept>
#include "uniformBuffers.hpp"

class DescriptorManager {
public:
    DescriptorManager(VkDevice device, std::vector<VkBuffer> buffers);
    ~DescriptorManager() = default;

    void destroyDescriptorManager();

    VkDescriptorSetLayout getDescriptorSetLayout() const;

    const VkDescriptorSet& getDescriptorSet(uint32_t index) const;

    void createDescriptorSets(const std::array<VkImageView, 4>& imageViews,
        const std::array<VkSampler, 4>& sampler);

private:
    VkDevice m_device;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSetLayout m_descriptorSetLayout;
    std::vector<VkDescriptorSet> m_descriptorSets;
    std::vector<VkBuffer> m_uniformBuffers;

    void createDescriptorSetLayout();
    void createDescriptorPool();
};
