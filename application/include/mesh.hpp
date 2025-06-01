#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include "bufferUtils.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
/**
 * @class Mesh
 * @brief Handles loading, buffering, and rendering of 3D mesh data.
 */
class Mesh {
public:
	Mesh() = default;
	/**
	 * @brief Constructs a mesh by loading a model from file and setting up Vulkan buffers.
	 * @param device Vulkan logical device.
	 * @param physicalDevice Vulkan physical device.
	 * @param commandPool Command pool for buffer command submissions.
	 * @param graphicsQueue Graphics queue to submit buffer commands.
	 * @param path File path to the model.
	 */
	Mesh(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const std::string& path);
	
	void freeMemory();
	/**
   * @brief Binds vertex and index buffers to a command buffer for rendering.
   * @param commandBuffer Command buffer to record binding commands.
   */
	void bindBuffers(VkCommandBuffer commandBuffer);

	/**
	 * @brief Issues draw call for the mesh.
	 * @param commandBuffer Command buffer to record draw commands.
	 */
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

	/**
   * @brief Loads model data from file.
   * @param path Path to the model file.
   */
	void loadModel(const std::string& path);
	/**
	 * @brief Processes nodes in the model scene graph.
	 * @param node Node to process.
	 * @param scene Pointer to the full scene data.
	 */
	void processNode(aiNode* node, const aiScene* scene);

	/**
	 * @brief Processes a mesh object from the scene, extracting vertices and indices.
	 * @param mesh Pointer to the mesh data.
	 * @param scene Pointer to the full scene data.
	 */
	void processMesh(aiMesh* mesh, const aiScene* scene);

};
