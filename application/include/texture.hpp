#pragma once

#include <vulkan/vulkan.h>
#include "bufferUtils.hpp"
#include "imageUtils.hpp"
#include <stb_image.h>

class Texture {

	public:
		Texture() = default; // default constructor

		Texture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, const char* path);

		void destroyTexture();

		VkImageView getTextureImageView() const {
			return m_textureImageView; // getter for the texture image view
		}
		VkSampler getTextureSampler() const {
			return m_textureSampler; // getter for the texture sampler
		}

	private:
		VkDevice m_device;
		VkPhysicalDevice m_physicalDevice;
		VkCommandPool m_commandPool;
		VkQueue m_queue;

		const char* m_texturePath;

		VkImage m_textureImage;
		VkDeviceMemory m_textureImageMemory;

		VkImageView m_textureImageView; // getter
		VkSampler m_textureSampler; // getter 

		void createTextureImage();
		

		void createTextureImageView();

		void createTextureSampler();
};