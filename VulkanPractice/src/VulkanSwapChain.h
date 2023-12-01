#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanSwapChainSupportDetails.h"
#include "VulkanQueueFamilyIndices.h"

namespace VulkanRenderer
{


	class VulkanSwapChain
	{
	private:		
		VkDevice* _device;
		
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(GLFWwindow& window, const VkSurfaceCapabilitiesKHR& capabilities);

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		void CreateImageViews();


	public:
		VkSwapchainKHR SwapChain;
		std::vector<VkImage> SwapChainImages;
		VkFormat SwapChainImageFormat;
		VkExtent2D SwapChainExtent;
		std::vector<VkImageView> SwapChainImageViews;

		VulkanSwapChain(GLFWwindow& window,
						VkDevice& device,
						VkSurfaceKHR& surface,
						VulkanSwapChainSupportDetails& swapChainSupport,
						VulkanQueueFamilyIndices& indices);
		~VulkanSwapChain();
	};




}