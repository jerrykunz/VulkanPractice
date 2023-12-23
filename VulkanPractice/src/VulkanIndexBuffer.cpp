#include "VulkanIndexBuffer.h"

namespace VulkanRenderer
{
    VulkanIndexBuffer::VulkanIndexBuffer()
    {

    }

    void VulkanIndexBuffer::LoadIndices(std::vector<uint32_t>& indices, VkPhysicalDevice& physicalDevice, VkDevice& device, VkQueue& graphicsQueue, VkCommandPool& commandPool)
    {
        Size = indices.size();
        VkDeviceSize bufferSize = sizeof(indices[0]) * Size;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IndexBuffer, IndexBufferMemory);

        CopyBuffer(device, graphicsQueue, commandPool, stagingBuffer, IndexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void VulkanIndexBuffer::Dispose(VkDevice& device)
    {
        vkDestroyBuffer(device, IndexBuffer, nullptr);
        vkFreeMemory(device, IndexBufferMemory, nullptr);
    }
}