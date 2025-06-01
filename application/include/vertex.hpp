#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>
/**
 * @struct Vertex
 * @brief Represents a single vertex with position, normal, texture coordinates, and tangent data.
 *
 * This struct is used to define the layout of vertex data passed to the vertex shader.
 */
struct Vertex {
	glm::vec3 pos; ///< Position of the vertex in 3D space.
	glm::vec3 normal; ///< Normal vector for lighting calculations.
	glm::vec2 texCoord; ///< Texture coordinates for mapping textures onto the geometry.
	glm::vec3 tangent; ///< Tangent vector for normal mapping, used in PBR lighting calculations.

	/**
	 * @brief Returns the vertex input binding description.
	 *
	 * Describes how data is read from the vertex buffer.
	 * @return A VkVertexInputBindingDescription struct.
	 */
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // move to next data entry after each vertex
		return bindingDescription;
	}
	/**
	 * @brief Returns the vertex input attribute descriptions.
	 *
	 * Maps vertex data to shader input locations.
	 * @return An array of VkVertexInputAttributeDescription structs.
	 */
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{}; // two because we are passing a vec2 and a vec3
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // 3 floats
		attributeDescriptions[0].offset = offsetof(Vertex, pos); // offset of the position data

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // 3 floats
		attributeDescriptions[1].offset = offsetof(Vertex, normal); // offset of the color data

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord); // texture coordinates

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT; // 3 floats for tangent
		attributeDescriptions[3].offset = offsetof(Vertex, tangent); // offset of the tangent data

		return attributeDescriptions;
	}
};