#pragma once

#include "Mesh.hpp"
#include "Texture.hpp"


class Model {
public:
    Model(VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandPool commandPool,
        VkQueue graphicsQueue,
        const std::string& modelPath,
        const char* texturePath)
        : m_device(device),
		m_physicalDevice(physicalDevice),
		m_commandPool(commandPool),
		m_graphicsQueue(graphicsQueue),
        m_mesh(device, physicalDevice, commandPool, graphicsQueue, modelPath),
        m_texture(device, physicalDevice, commandPool, graphicsQueue, texturePath)
    {

    }

    void loadTextures(const std::array<const char*, 4>& texturePaths) {
        for (size_t i = 0; i < 4; ++i) {
            m_textures[i] = Texture(
                m_device, m_physicalDevice, m_commandPool, m_graphicsQueue, texturePaths[i]
            );
        }
    }

    void destroyModel() {
        m_mesh.freeMemory();
        m_texture.destroyTexture();
        for (auto& tex : m_textures) {
            tex.destroyTexture();
        }
    }

    void bind(VkCommandBuffer commandBuffer) {
        m_mesh.bindBuffers(commandBuffer);
    }

    void draw(VkCommandBuffer commandBuffer) {
        m_mesh.draw(commandBuffer);
    }

    VkImageView getImageView() const { return m_texture.getTextureImageView(); }
    VkSampler getSampler() const { return m_texture.getTextureSampler(); }


    std::array<VkImageView, 4> getImageViews() const {
        std::array<VkImageView, 4> views{};
        for (size_t i = 0; i < 4; ++i) {
            views[i] = m_textures[i].getTextureImageView();
        }
        return views;
    }

    std::array<VkSampler, 4> getSamplers() const {
        std::array<VkSampler, 4> samplers{};
        for (size_t i = 0; i < 4; ++i) {
            samplers[i] = m_textures[i].getTextureSampler();
        }
        return samplers;
    }

private:
    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;
    VkCommandPool m_commandPool;
    VkQueue m_graphicsQueue;

    Mesh m_mesh;
	Texture m_texture;
    std::array<Texture, 4> m_textures;
};