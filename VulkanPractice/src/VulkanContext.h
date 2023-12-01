#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <iostream>
#include <fstream>
#include <array>
#include <vector>


#include "VulkanPhysicalDevice.h"

#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanFrameBuffer.h"
#include "VulkanVertex.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanUniformBuffer.h"
#include "VulkanImage.h"
#include "VulkanModel.h"



namespace VulkanRenderer
{
	class VulkanContext
	{
	private:
		bool _enableValidationLayers;
		const std::vector<const char*> _validationLayers = { "VK_LAYER_KHRONOS_validation" };
		int _maxFramesInFlight;
		uint32_t _currentFrame = 0;


		VkDebugUtilsMessengerEXT _debugMessenger;	

		//Wrappers
		VulkanPhysicalDevice* _physicalDevice;
		
		VulkanSwapChain* _swapChain;
		VulkanFrameBuffer* _frameBuffer;
		
		VulkanUniformBuffer* _uniformBuffer;		

		std::vector<VulkanImage*> _images;

		//Vulkan structs
		VkRenderPass _renderPass;
		VkDescriptorSetLayout _descriptorSetLayout;

		VkPipelineLayout _pipelineLayout;
		VkPipeline _graphicsPipeline;

		VkCommandPool _commandPool;


		VkDescriptorPool _descriptorPool;

		//VkDescriptorSetLayout _descriptorSetLayout;
		std::vector<VkDescriptorSet> _descriptorSets;

		std::vector<VkCommandBuffer> _commandBuffers;

		std::vector<VkSemaphore> _imageAvailableSemaphores;
		std::vector<VkSemaphore> _renderFinishedSemaphores;
		std::vector<VkFence> _inFlightFences;


		//INSTANCE
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		void InitializeInstance(const std::string& applicationName, const std::string& engineName);
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		//DEBUG MESSENGER
		void InitializeDebugMessenger();
		VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);
		void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator);

		//Render pass
		void CreateRenderPass(VulkanPhysicalDevice* physicalDevice,
							  VulkanDevice* device,
							  VkFormat& swapChainImageFormat);
		VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);
		VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice,
									 const std::vector<VkFormat>& candidates,
									 VkImageTiling tiling,
									 VkFormatFeatureFlags features);

		//DescriptorSetLayout
		void CreateDescriptorSetLayout();

		//Shaders
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		//Gfxpipeline
		void CreateGraphicsPipeline();

		//cmdpool
		void CreateCommandPool();

		//Descriptorpool
		void CreateDescriptorPool();

		//DescriptorSets
		void CreateDescriptorSets();

		//Commandbuffs
		void CreateCommandBuffers();

		//syncobj
		void CreateSyncObjects();

		void RecreateSwapChain(GLFWwindow* window);

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	public:		
		bool FrameBufferResized;

		VkInstance Instance;
		VkSurfaceKHR Surface;
		VulkanDevice* Device;

		VulkanVertexBuffer* VertexBuffer;
		VulkanIndexBuffer* IndexBuffer;

		VulkanContext(GLFWwindow* window, const std::string& applicationName, const std::string& engineName);
		~VulkanContext();

		void DrawFrame(GLFWwindow* window);

		void CleanUp();

		//SURFACE
		void CreateSurface(GLFWwindow* window);
	};



	
}