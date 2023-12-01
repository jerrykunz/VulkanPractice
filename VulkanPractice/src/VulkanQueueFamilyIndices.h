#pragma once
#include <optional>

namespace VulkanRenderer
{
	struct VulkanQueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() 
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};
}