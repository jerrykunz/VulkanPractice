#pragma once

#include "VulkanBuffer.h"
#include <vector>
#include "VulkanVertex.h"

namespace VulkanRenderer
{
	class VulkanIndexBuffer : public VulkanBuffer
	{
	public:
		VkBuffer IndexBuffer;
		VkDeviceMemory IndexBufferMemory;
		size_t Size;

		VulkanIndexBuffer();
		void LoadIndices(std::vector<uint32_t>& indices, VkPhysicalDevice& physicalDevice, VkDevice& device, VkQueue& graphicsQueue, VkCommandPool& commandPool);
		void Dispose(VkDevice& device);
	};
}
