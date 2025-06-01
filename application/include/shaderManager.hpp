#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>
#include <fstream>
/**
 * @namespace ShaderManager
 * @brief Utility functions for Vulkan shader module management.
 */
namespace ShaderManager {
	/**
	* @brief Creates a Vulkan shader module from SPIR-V bytecode.
	*
	* @param code A vector containing the SPIR-V bytecode.
	* @param device The Vulkan logical device handle.
	* @return VkShaderModule The created Vulkan shader module.
	*
	* @throws std::runtime_error if shader module creation fails.
	*/
	static VkShaderModule createShaderModule(const std::vector<char>& code, const VkDevice& device) {

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
		return shaderModule;
	}
	/**
	 * @brief Destroys a Vulkan shader module.
	 *
	 * @param shaderModule Reference to the shader module to destroy.
	 * @param logicalDevice The Vulkan logical device handle.
	 */
	static void destroyShaderModule(VkShaderModule& shaderModule,const VkDevice& logicalDevice) {
		vkDestroyShaderModule(logicalDevice, shaderModule, nullptr);
	}
	/**
	 * @brief Reads a binary file into a vector of bytes.
	 *
	 * @param filename The path to the file to read.
	 * @return std::vector<char> Vector containing the file's byte data.
	 *
	 * @throws std::runtime_error if the file cannot be opened.
	 */
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary); // open the file in binary mode at end of file

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		size_t fileSize = (size_t)file.tellg(); // get the size of the file
		std::vector<char> buffer(fileSize); // create a buffer of the size of the file

		file.seekg(0); // go back to the beginning of the file
		file.read(buffer.data(), fileSize); // read the file into the buffer

		file.close();
		return buffer;
	}
}