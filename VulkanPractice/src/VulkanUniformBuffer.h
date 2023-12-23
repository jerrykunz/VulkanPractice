#pragma once
#include "VulkanBuffer.h"
#include <vector>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include "Pool.h"

namespace VulkanRenderer
{
	struct ModelObject
	{
		alignas(16) glm::mat4 Transform;
	};

	//You can (and should) probably have multiple UBOs for different purposes
	struct UniformBufferObject
	{
		//alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		ModelObject models[100];
	};

	class VulkanUniformBuffer : public VulkanBuffer
	{
	private:
		VkDevice* _device;
		int _maxFramesInFlight;
		std::vector<FreeChunk> _freeChunks;
		int _modelIndex;

		static const int MAX_MODELS = 100;

	public:
		std::vector<VkBuffer> UniformBuffers;
		std::vector<VkDeviceMemory> UniformBuffersMemory;
		std::vector<UniformBufferObject*> UniformBuffersMapped;


		VulkanUniformBuffer(VkPhysicalDevice& physicalDevice, VkDevice& device, int maxFramesInFlight);
		~VulkanUniformBuffer();
		void Update(uint32_t currentImage, VkExtent2D swapChainExtent);
		uint32_t AddTransform();
		void RemoveTransform(uint32_t index);
		glm::mat4 GetTransform(uint32_t index, int frameIndex);
		void SetTransform(uint32_t index, glm::mat4 transform, int frameIndex);
	};
}
