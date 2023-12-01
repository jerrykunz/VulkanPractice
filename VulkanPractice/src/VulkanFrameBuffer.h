#pragma once
#include <array>
#include <vector>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "VulkanPhysicalDevice.h"

namespace VulkanRenderer
{

	class VulkanFrameBuffer
	{
	private:
		//std::vector<VkImageView> _swapChainImageViews;
		

		VkDevice* _device;

		

		VkFormat _colorFormat;
		VkFormat _depthFormat;

		void CreateColorResources(VulkanPhysicalDevice& physicalDevice, VkDevice& device, VkFormat& swapChainImageFormat, VkExtent2D& swapChainExtent);

		void CreateDepthResources(VulkanPhysicalDevice& physicalDevice, VkDevice& device, VkFormat& swapChainImageFormat, VkExtent2D& swapChainExtent);
		VkFormat FindDepthFormat(VkPhysicalDevice& physicalDevice);

		VkFormat FindSupportedFormat(VkPhysicalDevice& physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkPhysicalDevice& physicalDevice, VkDevice& device);		
		VkImageView CreateImageView(VkDevice& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

		uint32_t FindMemoryType(VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);


	public:
		std::vector<VkFramebuffer> SwapChainFramebuffers;

		VkImage ColorImage;
		VkDeviceMemory ColorImageMemory;
		VkImageView ColorImageView;

		VkImage DepthImage;
		VkDeviceMemory DepthImageMemory;
		VkImageView DepthImageView;

		VulkanFrameBuffer(VulkanPhysicalDevice& physicalDevice,
					      VkDevice& device,
					      VkRenderPass& renderPass,
						  const std::vector<VkImageView>& swapChainImageViews,
					      VkExtent2D& swapChainExtent,
					      VkFormat& swapChainImageFormat);
		~VulkanFrameBuffer();


	};

}