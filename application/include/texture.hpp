#pragma once

#include <vulkan/vulkan.h>
#include "bufferUtils.hpp"
#include "imageUtils.hpp"
#include <stb_image.h>

/**
 * @class Texture
 * @brief Handles Vulkan texture loading, image creation, view and sampler setup.
 */
class Texture {

	public:
		
		Texture() = default; // default constructor
		/**
	 * @brief Constructs a Texture object and initializes the texture resources.
	 * @param device Vulkan logical device.
	 * @param physicalDevice Vulkan physical device.
	 * @param commandPool Command pool for submitting copy/transition commands.
	 * @param queue Vulkan queue for executing commands.
	 * @param path Path to the image file to load.
	 */
		Texture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, const char* path);
		/**
	 * @brief Frees texture-related Vulkan resources including image, memory, image view, and sampler.
	 */
		void destroyTexture();

		VkImageView getTextureImageView() const {
			return m_textureImageView; // getter for the texture image view
		}
		VkSampler getTextureSampler() const {
			return m_textureSampler; // getter for the texture sampler
		}

	private:
		VkDevice m_device; ///< Vulkan logical device handle.
		VkPhysicalDevice m_physicalDevice; ///< Vulkan physical device handle.
		VkCommandPool m_commandPool; ///< Command pool for submitting commands.
		VkQueue m_queue; ///< Vulkan queue for executing commands.

		const char* m_texturePath; ///< Path to the texture image file.

		VkImage m_textureImage; // Vulkan image handle for the texture.
		VkDeviceMemory m_textureImageMemory; // Memory allocated for the texture image.

		VkImageView m_textureImageView; // Vulkan image view for the texture image, used for sampling in shaders.
		VkSampler m_textureSampler; // Vulkan sampler for the texture, defines how the texture is sampled in shaders.

		/**
		* @brief Loads the image from disk and creates a Vulkan image from it.
		*        This includes staging buffer creation and memory transfers.
		* @throws std::runtime_error If image loading or Vulkan resource creation fails.
		*/
		void createTextureImage();
		
		/**
 * @brief Creates an image view for the texture image.
 */
		void createTextureImageView();
		/**
   * @brief Creates a Vulkan sampler object for the texture.
   */
		void createTextureSampler();
};