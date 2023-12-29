#pragma once
#include "VulkanModel.h"

class GameObject
{
public:
	glm::mat4 Transform;
	VulkanRenderer::VulkanModel* Model;
	float RotationMultiplier;

	//GameObject();
	void Update();
	void Render();
};