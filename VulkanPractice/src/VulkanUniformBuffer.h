#pragma once
#include "VulkanBuffer.h"
#include <vector>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace VulkanRenderer
{
	struct UniformBufferObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	class VulkanUniformBuffer : public VulkanBuffer
	{
	private:
		VkDevice* _device;
		int _maxFramesInFlight;

	public:
		std::vector<VkBuffer> UniformBuffers;
		std::vector<VkDeviceMemory> UniformBuffersMemory;
		std::vector<void*> UniformBuffersMapped;

		VulkanUniformBuffer(VkPhysicalDevice& physicalDevice, VkDevice& device, int maxFramesInFlight);
		~VulkanUniformBuffer();
		void Update(uint32_t currentImage, VkExtent2D swapChainExtent);
	};
}
