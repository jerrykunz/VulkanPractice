#pragma once

#include "VulkanBuffer.h"
#include <vector>
#include "VulkanVertex.h"

namespace VulkanRenderer
{
	class VulkanIndexBuffer : public VulkanBuffer
	{
	private:
		VkDevice* _device;
		
		VkDeviceMemory _indexBufferMemory;
		

	public:
		VkBuffer IndexBuffer;
		std::vector<uint32_t> Indices;

		VulkanIndexBuffer(VkDevice& device);
		void LoadIndices(VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, VkCommandPool& commandPool);
		~VulkanIndexBuffer();
	};
}
