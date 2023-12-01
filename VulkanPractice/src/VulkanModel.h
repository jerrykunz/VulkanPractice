#pragma once
#include <vector>
#include <unordered_map>

#include "VulkanVertex.h"
#include <string>
#include <stdexcept>

namespace VulkanRenderer
{
	class VulkanModel
	{
	public:
		std::vector<Vertex> Vertices;
		std::vector<uint32_t> Indices;

		VulkanModel(const std::string& path);
	};
}