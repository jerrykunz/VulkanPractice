#include "VulkanVertexBuffer.h"

namespace VulkanRenderer
{
	VulkanVertexBuffer::VulkanVertexBuffer(VkDevice& device)
	{
        _device = &device;
	}

    void VulkanVertexBuffer::LoadVertices(VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, VkCommandPool& commandPool)
    {
        VkDeviceSize bufferSize = sizeof(Vertices[0]) * Vertices.size();

        //program -> stagingbuffer -> vertexbuffer
        //this is somehow more optimal, very weird as it is, something to do with cpu->gpu transfer bottleneck or w/e

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(physicalDevice, *_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(*_device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, Vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(*_device, stagingBufferMemory);

        CreateBuffer(physicalDevice, *_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VertexBuffer, _vertexBufferMemory);

        CopyBuffer(*_device, graphicsQueue, commandPool, stagingBuffer, VertexBuffer, bufferSize);

        vkDestroyBuffer(*_device, stagingBuffer, nullptr);
        vkFreeMemory(*_device, stagingBufferMemory, nullptr);
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        vkDestroyBuffer(*_device, VertexBuffer, nullptr);
        vkFreeMemory(*_device, _vertexBufferMemory, nullptr);
    }
}