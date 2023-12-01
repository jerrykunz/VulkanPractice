#pragma once
#include <stdexcept>

#include <vulkan/vulkan_core.h>

namespace VulkanRenderer
{
	class VulkanBuffer
	{
	protected:
		void CreateBuffer(VkPhysicalDevice& physicalDevice,
						  VkDevice& device,
						  VkDeviceSize size, 
						  VkBufferUsageFlags usage,
						  VkMemoryPropertyFlags properties,
						  VkBuffer& buffer,
						  VkDeviceMemory& bufferMemory);

		//used elsewhere
		uint32_t FindMemoryType(VkPhysicalDevice& physicalDevice,
								uint32_t typeFilter,
								VkMemoryPropertyFlags properties);

		void CopyBuffer(VkDevice& device,
						VkQueue graphicsQueue,
						VkCommandPool& commandPool,
						VkBuffer srcBuffer,
						VkBuffer dstBuffer, 
						VkDeviceSize size);

		//used elsewhere
		VkCommandBuffer BeginSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool);
		void EndSingleTimeCommands(VkDevice& device, VkQueue& graphicsQueue, VkCommandPool& commandPool, VkCommandBuffer& commandBuffer);
	};
}