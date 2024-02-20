#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <iostream>
#include <fstream>
#include <array>
#include <vector>


#include "VulkanPhysicalDevice.h"
#include "VulkanPushConstants.h"
#include "VulkanPipeLine.h"

#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanFrameBuffer.h"
#include "VulkanVertex.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanUniformBuffer.h"
#include "VulkanImage.h"
#include "VulkanModel.h"
#include "Input.h"
#include <glm/gtx/matrix_decompose.hpp>


namespace VulkanRenderer
{
	class VulkanContext
	{
	private:
		bool _enableValidationLayers;
		const std::vector<const char*> _validationLayers = { "VK_LAYER_KHRONOS_validation" };
		int _maxFramesInFlight;

		const uint32_t _maxTextures = 32;
		uint32_t _textureIndex = 0;
		std::vector<VulkanImage*> _textureSlots;

		VkDebugUtilsMessengerEXT _debugMessenger;	

		//Wrappers
		VulkanSwapChain* _swapChain;
		VulkanFrameBuffer* _frameBuffer;		

		//Vulkan structs
		VkRenderPass _renderPass;

		//VkPipelineLayout _pipelineLayout2DQuad;
		//VkPipeline _GraphicsPipeline2DQuad;

		VkDescriptorSetLayout _descriptorSetLayout;
		VkDescriptorPool _descriptorPool;
		std::vector<VkDescriptorSet> _descriptorSets;

		std::vector<VkCommandBuffer> _commandBuffers;

		std::vector<VkSemaphore> _imageAvailableSemaphores;
		std::vector<VkSemaphore> _renderFinishedSemaphores;
		std::vector<VkFence> _inFlightFences;

		VulkanPipeline* _quadPipeline;
		VulkanPipeline* _linePipeline;

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
		void CreateDescriptorSetLayout(/*const std::vector<VulkanUniformBuffer*>& uniformBuffers*/);

		//Shaders
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		//Gfxpipeline
		//void CreateGraphicsPipeline2DQuad();

		//cmdpool
		void CreateCommandPool();

		//Descriptorpool
		void CreateDescriptorPool(/*const std::vector<VulkanUniformBuffer*>& uniformBuffers*/);


		//Commandbuffs
		void CreateCommandBuffers();

		//syncobj
		void CreateSyncObjects();

		void RecreateSwapChain(GLFWwindow* window);

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		//glm::mat4 CreateViewMatrix(float pitch, float yaw, float roll, glm::vec3 position);

	public:
		uint32_t CurrentFrame = 0;
		bool FrameBufferResized;

		

		VkInstance Instance;
		VkSurfaceKHR Surface;

		VkCommandPool CommandPool;

		VulkanDevice* Device;
		VulkanPhysicalDevice* PhysicalDevice;

		VulkanUniformBuffer* ViewProjectionUniformBuffer;
		VulkanUniformBuffer* InstanceDataUniformBuffer;

		std::vector<VulkanUniformBuffer*> UniformBuffers;
		std::vector<VulkanImage*> Images;

		//Quad rendering
		glm::vec4 _quadVertexPositions[4];

		std::vector<VulkanVertexBuffer> QuadVertexBuffer;
		std::vector<QuadVertex*> QuadVertices;
		uint32_t QuadVertexCount;
		uint32_t QuadIndexCount;


		VulkanIndexBuffer QuadIndexBuffer;

		uint32_t _maxQuadVertices;
		uint32_t _maxQuadIndices;

		VulkanImage* WhiteTexture;

		//Line rendering
		std::vector<VulkanVertexBuffer> LineVertexBuffer;
		std::vector<LineVertex*> LineVertices;
		uint32_t LineVertexCount;
		uint32_t LineIndexCount;

		VulkanIndexBuffer LineIndexBuffer;

		uint32_t _maxLineVertices;
		uint32_t _maxLineIndices;


		VulkanContext(GLFWwindow* window, const std::string& applicationName, const std::string& engineName);
		~VulkanContext();

		void InitQuadRendering();
		void InitLineRendering();

		//DescriptorSets
		void CreateDescriptorSets();
		void UpdateTextureDescriptorSets();

		void RenderQuad(const glm::mat4& transform, const glm::vec4& color);
		void RenderQuad(const glm::mat4& transform, VulkanImage& texture, float tilingFactor, const glm::vec4& tintColor, glm::vec2 uv0, glm::vec2 uv1);
		void RenderLine(const glm::vec3 p1, const glm::vec3 p2, const glm::vec4& color1, const glm::vec4& color2);

		void DrawFrame(GLFWwindow* window);

		void CleanUp();

		//SURFACE
		void CreateSurface(GLFWwindow* window);

		ViewProjectionUBO* GetViewProjectionUBO();
		InstanceDataUBO* GetInstanceDataUBO();
	};



	
}