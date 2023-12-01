#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <stdexcept>
#include <optional>
#include "VulkanQueueFamilyIndices.h"
#include "VulkanSwapChainSupportDetails.h"

namespace VulkanRenderer
{
	class VulkanPhysicalDevice
	{		
	private:		
				
		
		bool IsDeviceSuitable(const VkPhysicalDevice& device, VkSurfaceKHR& surface);
		VkSampleCountFlagBits GetMaxUsableSampleCount();
		VulkanQueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& device, VkSurfaceKHR& surface);
		bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device);
		VulkanSwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice& device, VkSurfaceKHR& surface);
	public:
		VkPhysicalDevice Device = VK_NULL_HANDLE;
		VulkanQueueFamilyIndices QueueIndices;
		//VulkanSwapChainSupportDetails SwapChainDetails;
		const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkSampleCountFlagBits MsaaSamples = VK_SAMPLE_COUNT_1_BIT;

		VulkanPhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface);
		VulkanSwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR& surface);

	};



}