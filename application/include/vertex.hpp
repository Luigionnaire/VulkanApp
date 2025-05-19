#pragma once

#include <vulkan/vulkan.h>

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // move to next data entry after each vertex
		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{}; // two because we are passing a vec2 and a vec3
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // 2 floats
		attributeDescriptions[0].offset = offsetof(Vertex, pos); // offset of the position data

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // 3 floats
		attributeDescriptions[1].offset = offsetof(Vertex, color); // offset of the color data

		return attributeDescriptions;
	}
};