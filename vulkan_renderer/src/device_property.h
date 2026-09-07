#pragma once

#include <render_id.h>

#include <vulkan/vulkan_core.h>

#include <vector>
#include <optional>


namespace render
{
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() const 
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}

	std::optional<uint32_t> getIndex() const
	{
		if (graphicsFamily.has_value())
		{
			return graphicsFamily;
		}
		if (presentFamily.has_value())
		{
			return presentFamily;
		}
		return {};
	}
};

struct PhysicalDeviceProperties
{
	VkPhysicalDeviceFeatures device_features;
	std::vector<QueueFamilyIndices> queue_family_indices;
};

}
