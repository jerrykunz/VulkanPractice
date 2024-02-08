#pragma once

#include <vulkan/vulkan_core.h>

//#ifndef STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION

//#endif

#include <cmath>
#include <iostream>
#include "VulkanDevice.h"
#include "VulkanUniformBuffer.h"
#include "VulkanImage.h"

namespace VulkanRenderer
{
	class VulkanImage
	{
	private:
		VkDevice* _device;		
		uint32_t _mipLevels;

		void CreateTextureSampler(VkPhysicalDevice& physicalDevice);
		void GenerateMipmaps(VkPhysicalDevice& physicalDevice, VulkanDevice& device, VkCommandPool& commandPool, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		void CopyBufferToImage(VulkanDevice& device, VkCommandPool& commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		VkCommandBuffer BeginSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool);
		void EndSingleTimeCommands(VulkanDevice& device, VkCommandPool& commandPool, VkCommandBuffer& commandBuffer);
		void TransitionImageLayout(VulkanDevice& device, VkCommandPool& commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
		void CreateImage(VkPhysicalDevice& physicalDevice, VkDevice& device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void CreateBuffer(VkPhysicalDevice& physicalDevice, VkDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		uint32_t FindMemoryType(VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	public:
		VkImage TextureImage;
		VkImageView TextureImageView;
		VkDeviceMemory TextureImageMemory;
		VkSampler TextureSampler;
		VkDescriptorImageInfo Descriptor;

		VulkanImage(const std::string& path, VkPhysicalDevice& physicalDevice, VulkanDevice& device, VkCommandPool& commandPool, VkImageLayout imageLayout);
		VulkanImage(uint32_t* color, VkPhysicalDevice& physicalDevice, VulkanDevice& device, VkCommandPool& commandPool, VkImageLayout imageLayout);
		void FinishImage(unsigned char* pixels, int texWidth, int texHeight, VkPhysicalDevice& physicalDevice, VulkanDevice& device, VkCommandPool& commandPool, VkImageLayout imageLayout);
		~VulkanImage();
	};


}