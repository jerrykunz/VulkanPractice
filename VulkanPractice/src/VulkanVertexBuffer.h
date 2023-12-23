#pragma once

#include "VulkanBuffer.h"
#include <vector>
#include "VulkanVertex.h"

namespace VulkanRenderer
{
	class VulkanVertexBuffer : public VulkanBuffer
	{
	public:
		VkBuffer VertexBuffer;
		VkDeviceMemory VertexBufferMemory;

		VulkanVertexBuffer();
		void LoadVertices(std::vector<Vertex>& vertices, VkPhysicalDevice& physicalDevice, VkDevice& device, VkQueue& graphicsQueue, VkCommandPool& commandPool);
		void Dispose(VkDevice& device);
	};
}
