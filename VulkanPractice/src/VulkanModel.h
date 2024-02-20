#pragma once
#include <vector>
#include <unordered_map>

#include "VulkanVertex.h"
#include <string>
#include <stdexcept>
#include "VulkanVertexBuffer.h"

#include <tiny_obj_loader.h>
#include "VulkanDevice.h"
#include "VulkanIndexBuffer.h"
#include "VulkanImage.h"

namespace VulkanRenderer
{
	class VulkanModel
	{
	public:
		/*uint32_t TransformIndex;
		glm::mat4 Transform;*/
		//std::vector<InstanceData> Instances;
		
		uint8_t instanceCount;
		uint8_t instancesIndex;
		uint8_t instancesEnd;

		std::vector<QuadVertex> Vertices;
		VulkanVertexBuffer VertexBuffer;

		std::vector<uint32_t> Indices;
		VulkanIndexBuffer IndexBuffer;

		VulkanImage* Texture;

		//might need descriptorset later

		VulkanModel(const std::string& path, VulkanImage& image/*, VulkanUniformBuffer& uniformBuffer*/, VkPhysicalDevice& physicalDevice, VkDevice& device, VkQueue& graphicsQueue, VkCommandPool& commandPool);

		//void AddInstanceData(glm::mat4 transform);

		void Dispose(VkDevice& device, VulkanUniformBuffer& uniformBuffer);

		uint8_t GetFirstInstanceIndex();
	};
}