#include "VulkanIndexBuffer.h"

namespace VulkanRenderer
{
    VulkanIndexBuffer::VulkanIndexBuffer(VkDevice& device)
    {
        _device = &device;       
    }

    void VulkanIndexBuffer::LoadIndices(VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, VkCommandPool& commandPool)
    {
        VkDeviceSize bufferSize = sizeof(Indices[0]) * Indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(physicalDevice, *_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(*_device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, Indices.data(), (size_t)bufferSize);
        vkUnmapMemory(*_device, stagingBufferMemory);

        CreateBuffer(physicalDevice, *_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IndexBuffer, _indexBufferMemory);

        CopyBuffer(*_device, graphicsQueue, commandPool, stagingBuffer, IndexBuffer, bufferSize);

        vkDestroyBuffer(*_device, stagingBuffer, nullptr);
        vkFreeMemory(*_device, stagingBufferMemory, nullptr);
    }

    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        vkDestroyBuffer(*_device, IndexBuffer, nullptr);
        vkFreeMemory(*_device, _indexBufferMemory, nullptr);
    }
}