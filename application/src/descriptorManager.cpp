#include "DescriptorManager.hpp"

DescriptorManager::DescriptorManager(VkDevice device, std::vector<VkBuffer> buffers)
    : m_device(device), m_uniformBuffers(std::move(buffers)) {
    createDescriptorSetLayout();
    createDescriptorPool();
}

void DescriptorManager::destroyDescriptorManager() {
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
}

VkDescriptorSetLayout DescriptorManager::getDescriptorSetLayout() const {
    return m_descriptorSetLayout;
}

const VkDescriptorSet& DescriptorManager::getDescriptorSet(uint32_t index) const {
    if (index < m_descriptorSets.size()) {
        return m_descriptorSets[index];
    }
    throw std::out_of_range("Index out of range for descriptor sets");
}

void DescriptorManager::createDescriptorSets(const std::array<VkImageView, 4>& imageViews,
    const std::array<VkSampler, 4>& sampler) {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UBO);

        std::array<VkWriteDescriptorSet, 5> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        std::array<VkDescriptorImageInfo, 4> imageInfos{};
        for (size_t j = 0; j < 4; ++j) {
            imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[j].imageView = imageViews[j];
            imageInfos[j].sampler = sampler[j];

            descriptorWrites[j + 1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j + 1].dstSet = m_descriptorSets[i];
            descriptorWrites[j + 1].dstBinding = static_cast<uint32_t>(j + 1);
            descriptorWrites[j + 1].dstArrayElement = 0;
            descriptorWrites[j + 1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[j + 1].descriptorCount = 1;
            descriptorWrites[j + 1].pImageInfo = &imageInfos[j];
        }

        vkUpdateDescriptorSets(m_device,
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(),
            0, nullptr);
    }
}

void DescriptorManager::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 5> bindings;
    bindings[0] = uboLayoutBinding;

    for (uint32_t i = 0; i < 4; ++i) {
        VkDescriptorSetLayoutBinding samplerBinding{};
        samplerBinding.binding = i + 1;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.descriptorCount = 1;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerBinding.pImmutableSamplers = nullptr;
        bindings[i + 1] = samplerBinding;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DescriptorManager::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 4);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}
#pragma once

