#include "VulkanUniformBuffer.h"

namespace VulkanRenderer
{
    VulkanUniformBuffer::VulkanUniformBuffer(VkPhysicalDevice& physicalDevice, VkDevice& device, int maxFramesInFlight) 
    {
        _device = &device;
        _maxFramesInFlight = maxFramesInFlight;
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        UniformBuffers.resize(_maxFramesInFlight);
        UniformBuffersMemory.resize(_maxFramesInFlight);
        UniformBuffersMapped.resize(_maxFramesInFlight);

        for (size_t i = 0; i < _maxFramesInFlight; i++)
        {
            CreateBuffer(physicalDevice,
                         device,
                         bufferSize, 
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         UniformBuffers[i],
                         UniformBuffersMemory[i]);

            vkMapMemory(device, UniformBuffersMemory[i], 0, bufferSize, 0, &UniformBuffersMapped[i]);
        }
    }

    VulkanUniformBuffer::~VulkanUniformBuffer()
    {
        for (size_t i = 0; i < _maxFramesInFlight; i++)
        {
            vkDestroyBuffer(*_device, UniformBuffers[i], nullptr);
            vkFreeMemory(*_device, UniformBuffersMemory[i], nullptr);
        }
    }

    void VulkanUniformBuffer::Update(uint32_t currentImage, VkExtent2D swapChainExtent)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        memcpy(UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }
}