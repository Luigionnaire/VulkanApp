#pragma once

#include <vulkan/vulkan.h>
#include "bufferUtils.hpp"
#include "imageUtils.hpp"
#include <stb_image.h>
class Texture {

	public:
		Texture(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue) : m_device(device), m_physicalDevice(physicalDevice), m_commandPool(commandPool), m_queue(queue) {
			createTextureImage();
			createTextureImageView();
			createTextureSampler();
		}

		void destroyTexture() {
			vkDestroySampler(m_device, m_textureSampler, nullptr); // destroy the sampler
			vkDestroyImageView(m_device, m_textureImageView, nullptr); // destroy the image view
			vkDestroyImage(m_device, m_textureImage, nullptr); // destroy the image
			vkFreeMemory(m_device, m_textureImageMemory, nullptr); // free the memory
		}
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


		VkImage m_textureImage;
		VkDeviceMemory m_textureImageMemory;

		VkImageView m_textureImageView; // getter
		VkSampler m_textureSampler; // getter 

		void createTextureImage() {
			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = stbi_load("./assets/textures/statue.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha); // TODO take this from constructor
			VkDeviceSize imageSize = texWidth * texHeight * 4;

			if (!pixels) {
				throw std::runtime_error("failed to load texture image!");
			}

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			BufferUtils::createBuffer(m_device, m_physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pixels, static_cast<size_t>(imageSize));
			vkUnmapMemory(m_device, stagingBufferMemory);

			stbi_image_free(pixels);

			createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);
			BufferUtils::transitionImageLayout(m_device, m_commandPool, m_queue, m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); // transition to transfer layout
			BufferUtils::copyBufferToImage(m_device, m_commandPool, m_queue, stagingBuffer, m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
			BufferUtils::transitionImageLayout(m_device, m_commandPool, m_queue, m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); // transition to shader layout

			vkDestroyBuffer(m_device, stagingBuffer, nullptr);
			vkFreeMemory(m_device, stagingBufferMemory, nullptr);

		}

		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.flags = 0; // for multisampling

			if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS)
			{
				throw std::runtime_error("falied to create image!");
			}

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(m_device, image, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = BufferUtils::findMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate image memory!");
			}

			vkBindImageMemory(m_device, image, imageMemory, 0);

		}

		void createTextureImageView() {
			m_textureImageView = ImageUtils::createImageView(m_device, m_textureImage, VK_FORMAT_R8G8B8A8_SRGB); // create the texture image view
		}


		void createTextureSampler() {
			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR; // linear filtering
			samplerInfo.minFilter = VK_FILTER_LINEAR; // linear filtering
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // repeat the texture
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; // repeat the texture
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; // repeat the texture

			samplerInfo.anisotropyEnable = VK_TRUE; // enable anisotropy

			VkPhysicalDeviceProperties properties{}; // can query at the start if used multiple times
			vkGetPhysicalDeviceProperties(m_physicalDevice, &properties); // get the physical device properties
			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // set it to the max allowed by device

			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // border color
			samplerInfo.unnormalizedCoordinates = VK_FALSE; // normalized coordinates
			samplerInfo.compareEnable = VK_FALSE; // no comparison
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // percentage-closer filtering

			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // linear mipmapping
			samplerInfo.mipLodBias = 0.0f; // no mipmap bias
			samplerInfo.minLod = 0.0f; // no min lod
			samplerInfo.maxLod = 0.0f; // no max lod

			if (vkCreateSampler(m_device, &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture sampler!");
			}
		}
};