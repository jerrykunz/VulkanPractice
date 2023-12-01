#include "VulkanDevice.h"



namespace VulkanRenderer
{
    VulkanDevice::VulkanDevice(bool enableValidationLayers,
                               const std::vector<const char*> validationLayers,
                               VulkanPhysicalDevice& physicalDevice)
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { physicalDevice.QueueIndices.graphicsFamily.value(), physicalDevice.QueueIndices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(physicalDevice.DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = physicalDevice.DeviceExtensions.data();

        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice.Device, &createInfo, nullptr, &Device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(Device, physicalDevice.QueueIndices.graphicsFamily.value(), 0, &GraphicsQueue);
        vkGetDeviceQueue(Device, physicalDevice.QueueIndices.presentFamily.value(), 0, &PresentQueue);
    }

    VulkanDevice::~VulkanDevice()
    {
        vkDestroyDevice(Device, nullptr);
    }
}