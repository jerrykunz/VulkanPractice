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
#include "Pool.h"

namespace VulkanRenderer
{
	class VulkanUniformBuffer2 : public VulkanBuffer
	{
	private:
		VkDevice* _device;

	public:
		std::vector<VkBuffer> UniformBuffers;
		std::vector<VkDeviceMemory> UniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;

		VulkanUniformBuffer2(VkDevice& device);
		~VulkanUniformBuffer2();
	};
}