#pragma once

#include "model.hpp"

Model::Model(VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkCommandPool commandPool,
    VkQueue graphicsQueue,
    const std::string& modelPath)
    : m_device(device),
    m_physicalDevice(physicalDevice),
    m_commandPool(commandPool),
    m_graphicsQueue(graphicsQueue),
    m_mesh(device, physicalDevice, commandPool, graphicsQueue, modelPath)
{}

void Model::loadTextures(const std::array<const char*, 4>& texturePaths) {
    for (size_t i = 0; i < 4; ++i) {
        m_textures[i] = Texture(
            m_device, m_physicalDevice, m_commandPool, m_graphicsQueue, texturePaths[i]
        );
    }
}
void Model::destroyModel() {
    m_mesh.freeMemory();
    for (auto& tex : m_textures) {
        tex.destroyTexture();
    }
}

void Model::bind(VkCommandBuffer commandBuffer) {
    m_mesh.bindBuffers(commandBuffer);
}

void Model::draw(VkCommandBuffer commandBuffer) {
    m_mesh.draw(commandBuffer);
}


std::array<VkImageView, 4> Model::getImageViews() const {
    std::array<VkImageView, 4> views{};
    for (size_t i = 0; i < 4; ++i) {
        views[i] = m_textures[i].getTextureImageView();
    }
    return views;
}

std::array<VkSampler, 4> Model::getSamplers() const {
    std::array<VkSampler, 4> samplers{};
    for (size_t i = 0; i < 4; ++i) {
        samplers[i] = m_textures[i].getTextureSampler();
    }
    return samplers;
}