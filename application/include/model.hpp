#pragma once

#include "Mesh.hpp"
#include "Texture.hpp"

/**
 * @class Model
 * @brief Represents a 3D model composed of a mesh and multiple textures.
 */
class Model {
public:
    /**
     * @brief Constructs a Model with mesh and a single texture.
     * @param device Vulkan logical device.
     * @param physicalDevice Vulkan physical device.
     * @param commandPool Command pool for buffer/texture operations.
     * @param graphicsQueue Queue for command submissions.
     * @param modelPath Path to the model file.
     * @param texturePath Path to the initial texture file.
     */
    Model(VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandPool commandPool,
        VkQueue graphicsQueue,
        const std::string& modelPath,
        const char* texturePath);
    
    /**
     * @brief Loads up to 4 textures from given file paths.
     * @param texturePaths Array of 4 texture file paths.
     */
    void loadTextures(const std::array<const char*, 4>& texturePaths);
    void destroyModel();

    /**
     * @brief Binds the mesh buffers to the given command buffer for rendering.
     * @param commandBuffer Command buffer to record commands.
     */
    void bind(VkCommandBuffer commandBuffer);

    /**
    * @brief Issues a draw call for the model's mesh.
    * @param commandBuffer Command buffer to record draw commands.
    */
    void draw(VkCommandBuffer commandBuffer);

    /// Returns image views for the additional 4 textures.
    std::array<VkImageView, 4> getImageViews() const;

    /// Returns samplers for the additional 4 textures.
    std::array<VkSampler, 4> getSamplers() const;
private:
	VkDevice m_device; ///< Vulkan logical device.
	VkPhysicalDevice m_physicalDevice; ///< Vulkan physical device.
	VkCommandPool m_commandPool; ///< Command pool for buffer/texture operations.
	VkQueue m_graphicsQueue; ///< Queue for command submissions.

	Mesh m_mesh; ///< Mesh representing the model.
	std::array<Texture, 4> m_textures; ///< Array of textures associated with the model.
};