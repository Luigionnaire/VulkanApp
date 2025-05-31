#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include "bufferUtils.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Mesh {
public:
	Mesh() = default;
	Mesh(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const std::string& path);
	void freeMemory();
	void bindBuffers(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);


private:
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkCommandPool m_commandPool;
	VkQueue m_graphicsQueue;

	std::vector<Vertex> m_vertices;
	std::vector<uint16_t> m_indices;

	VkBuffer m_vertexBuffer;
	VkBuffer m_indexBuffer;

	VkDeviceMemory m_vertexBufferMemory;
	VkDeviceMemory m_indexBufferMemory;

	void loadModel(const std::string& path);
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);

};
