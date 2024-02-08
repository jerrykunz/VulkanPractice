#pragma once
#include "VulkanModel.h"
#include "VulkanContext.h"
#include "VulkanUniformBuffer.h"

class GameObject
{
public:
	bool Visible;
	glm::mat4 Transform;
	VulkanRenderer::VulkanImage* Texture;
	glm::vec4 Color;
	float RotationMultiplier;

	//GameObject();
	void Update(float deltaTime);
	void Render(VulkanRenderer::VulkanContext& vulkanContext);
};
