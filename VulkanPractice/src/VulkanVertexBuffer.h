#pragma once

#include "VulkanBuffer.h"
#include <vector>
#include "VulkanVertex.h"

namespace VulkanRenderer
{
	class VulkanVertexBuffer : public VulkanBuffer
	{
	private:
		VkDevice* _device;		
		VkDeviceMemory _vertexBufferMemory;
		
	public:
		VkBuffer VertexBuffer;
		std::vector<Vertex> Vertices;

		VulkanVertexBuffer(VkDevice& device);
		void LoadVertices(VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, VkCommandPool& commandPool);
		~VulkanVertexBuffer();
	};
}
