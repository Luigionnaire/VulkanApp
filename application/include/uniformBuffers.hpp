#pragma once
#define GLM_FORCE_RADIANS
#include "vulkan/vulkan.h"
#include <vector>
#include <glm/glm.hpp>
#include "config.hpp"
#include "bufferUtils.hpp"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

/**
 * @struct UBO
 * @brief Represents a Uniform Buffer Object containing transformation matrices.
 *
 * This struct is used to pass transformation data to shaders.
 */
struct UBO {
	glm::mat4 model; ///< Model transformation matrix.
	glm::mat4 view; ///< View transformation matrix.
	glm::mat4 proj; ///< Projection transformation matrix.
};

/**
 * @class UniformBuffers
 * @brief Manages Vulkan uniform buffers for each frame in flight.
 *
 * This class handles creation, destruction, and updating of per-frame uniform buffers used to store transformation matrices.
 */
class UniformBuffers {
public:
	/**
	 * @brief Constructs and initializes uniform buffers.
	 * @param device Vulkan logical device.
	 * @param physicalDevice Vulkan physical device.
	 */
	UniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice);


	/**
	 * @brief Destroys all allocated uniform buffers and associated memory.
	 */
	void destroyUniformBuffers();

	/**
 * @brief Updates the uniform buffer for the given frame index with transformation matrices.
 * @param currentImage Index of the current swapchain image (frame in flight).
 * @param swapChainExtent Current swapchain extent (used to compute aspect ratio).
 */
	void updateUniformBuffer(uint32_t currentImage, VkExtent2D swapChainExtent);

	/**
	 * @brief Retrieves the Vulkan buffer for a given frame index.
	 * @param index Frame index.
	 * @return Vulkan uniform buffer.
	 * @throws std::out_of_range if the index is invalid.
	 */
	VkBuffer getUniformBuffer(uint32_t index) const;

	std::vector<VkBuffer> getUniformBuffers() const {
		return m_uniformBuffers;
	}
private:
	std::vector<VkBuffer> m_uniformBuffers; ///< Vector of Vulkan uniform buffers for each frame in flight.
	std::vector<VkDeviceMemory> m_uniformBuffersMemory; ///< Vector of memory allocated for each uniform buffer.
	std::vector<void*> m_uniformBuffersMapped; ///< Vector of mapped pointers to each uniform buffer's memory.
	VkDevice m_device; ///< Vulkan logical device handle.
	VkPhysicalDevice m_physicalDevice; ///< Vulkan physical device handle.
	UBO ubo; ///< Uniform Buffer Object containing transformation matrices.

	/**
 * @brief Allocates and maps uniform buffers for all frames in flight.
 */
	void createUniformBuffers();
};
