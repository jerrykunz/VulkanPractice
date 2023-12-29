#pragma once
#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanSwapChainSupportDetails.h"
#include "VulkanQueueFamilyIndices.h"

namespace VulkanRenderer
{
	class VulkanDescriptorSets
	{
	public:
		uint32_t _maxFramesInFlight;
		std::vector<VkDescriptorSetLayoutBinding> _layoutBindings;


		VulkanDescriptorSets(uint32_t maxFramesInFlight);

		void AddDescriptorSetLayoutBinding(VkDescriptorSetLayoutBinding binding);
		void AddDescriptorSetLayoutBinding(uint32_t binding,
										   uint32_t descriptorCount, 
										   VkDescriptorType descriptorType, 
										   VkSampler* pImmutableSamplers, 
										   VkShaderStageFlags stageFlags);


		void CreateDescriptorSets();
	};


}