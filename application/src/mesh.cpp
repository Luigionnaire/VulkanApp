#pragma once

#include "mesh.hpp"

Mesh::Mesh(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const std::string& path)
	: m_device(device),
	m_physicalDevice(physicalDevice),
	m_commandPool(commandPool),
	m_graphicsQueue(graphicsQueue)
{
	loadModel(path);

	BufferUtils::createVertexBuffer(m_device, m_physicalDevice, m_commandPool, m_graphicsQueue, m_vertices, m_vertexBuffer, m_vertexBufferMemory);
	BufferUtils::createIndexBuffer(m_device, m_physicalDevice, m_commandPool, m_graphicsQueue, m_indices, m_indexBuffer, m_indexBufferMemory);
}

void Mesh::freeMemory() {
	vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
	vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
	vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
	vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
}

void Mesh::bindBuffers(VkCommandBuffer commandBuffer) {
	//BUFFERS
	VkBuffer vertexBuffers[] = { m_vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

}

void Mesh::draw(VkCommandBuffer commandBuffer)
{
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
}


void Mesh::loadModel(const std::string& path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		throw std::runtime_error("Failed to load model: " + std::string(importer.GetErrorString()));
	}
	processNode(scene->mRootNode, scene);
}


void Mesh::processNode(aiNode* node, const aiScene* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(mesh, scene);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

void Mesh::processMesh(aiMesh* mesh, const aiScene* scene) {
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex{};

		vertex.pos = {
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		};

		if (mesh->HasNormals()) {
			vertex.normal = {
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z
			};
		}

		if (mesh->mTextureCoords[0]) {
			vertex.texCoord = {
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			};
		}
		else {
			vertex.texCoord = { 0.0f, 0.0f };
		}

		if (mesh->HasTangentsAndBitangents()) {
			vertex.tangent = {
				mesh->mTangents[i].x,
				mesh->mTangents[i].y,
				mesh->mTangents[i].z
			};
		}
		else {
			vertex.tangent = glm::vec3(0.0f, 0.0f, 0.0f); // fallback tangent
		}

		m_vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			m_indices.push_back(static_cast<uint16_t>(face.mIndices[j]));
		}
	}
}