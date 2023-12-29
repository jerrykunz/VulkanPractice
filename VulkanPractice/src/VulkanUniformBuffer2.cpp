#include "VulkanUniformBuffer2.h"

namespace VulkanRenderer
{
    VulkanUniformBuffer2::VulkanUniformBuffer2(VkDevice& device)
    {
        *_device = device;
    }

    VulkanUniformBuffer2::~VulkanUniformBuffer2()
    {
        size_t sz = UniformBuffers.size();
        for (size_t i = 0; i < sz; i++)
        {
            vkDestroyBuffer(*_device, UniformBuffers[i], nullptr);
            vkFreeMemory(*_device, UniformBuffersMemory[i], nullptr);
        }
    }

}