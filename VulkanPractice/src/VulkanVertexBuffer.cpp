#include "VulkanVertexBuffer.h"

namespace VulkanRenderer
{
	VulkanVertexBuffer::VulkanVertexBuffer()
	{
        VertexBuffer = VK_NULL_HANDLE;
	}

    //template<typename VertexType>
    //void VulkanVertexBuffer::LoadVertices(std::vector<VertexType>& vertices, VkPhysicalDevice& physicalDevice, VkDevice& device, VkQueue& graphicsQueue, VkCommandPool& commandPool)
    //{
    //    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    //    //program -> stagingbuffer -> vertexbuffer
    //    //this is somehow more optimal, very weird as it is, something to do with cpu->gpu transfer bottleneck or w/e

    //    VkBuffer stagingBuffer;
    //    VkDeviceMemory stagingBufferMemory;
    //    CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    //    void* data;
    //    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    //    memcpy(data, vertices.data(), (size_t)bufferSize);
    //    vkUnmapMemory(device, stagingBufferMemory);

    //    CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VertexBuffer, VertexBufferMemory);

    //    CopyBuffer(device, graphicsQueue, commandPool, stagingBuffer, VertexBuffer, bufferSize);

    //    vkDestroyBuffer(device, stagingBuffer, nullptr);
    //    vkFreeMemory(device, stagingBufferMemory, nullptr);
    //}

    //template<typename VertexType>
    //void VulkanVertexBuffer::LoadVertices(VertexType* vertices, size_t verticeCount, VkPhysicalDevice& physicalDevice, VkDevice& device, VkQueue& graphicsQueue, VkCommandPool& commandPool)
    //{
    //    VkDeviceSize bufferSize = sizeof(vertices[0]) * verticeCount;

    //    //program -> stagingbuffer -> vertexbuffer
    //    //this is somehow more optimal, very weird as it is, something to do with cpu->gpu transfer bottleneck or w/e

    //    VkBuffer stagingBuffer;
    //    VkDeviceMemory stagingBufferMemory;
    //    CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    //    void* data;
    //    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    //    memcpy(data, vertices, (size_t)bufferSize);
    //    vkUnmapMemory(device, stagingBufferMemory);

    //    CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VertexBuffer, VertexBufferMemory);

    //    CopyBuffer(device, graphicsQueue, commandPool, stagingBuffer, VertexBuffer, bufferSize);

    //    vkDestroyBuffer(device, stagingBuffer, nullptr);
    //    vkFreeMemory(device, stagingBufferMemory, nullptr);
    //}

    void VulkanVertexBuffer::Dispose(VkDevice& device)
    {
        vkDestroyBuffer(device, VertexBuffer, nullptr);
        vkFreeMemory(device, VertexBufferMemory, nullptr);
    }
}