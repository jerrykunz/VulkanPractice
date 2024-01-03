#pragma once
#include "VulkanModel.h"
#include "VulkanContext.h"
#include "VulkanUniformBuffer.h"

class GameObject
{
public:
	bool Visible;
	glm::mat4 Transform;
	VulkanRenderer::VulkanModel* Model;
	float RotationMultiplier;

	//GameObject();
	void Update();
	void Render(VulkanRenderer::InstanceDataUBO* ubo);
};
