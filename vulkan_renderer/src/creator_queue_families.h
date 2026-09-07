#pragma once

#include <optional>
#include <cstdint>
#include <vector>
#include <memory>

#include <vulkan/vulkan_core.h>

#include "struct_data.h"
#include "device_property.h"

namespace render
{

class CreatorQueueFamilies
{
public:
	[[nodiscard]] static std::vector<QueueFamilyIndices> CreateQueueFamilyIndices(
		std::shared_ptr<DataSurface> surface,
		const VkPhysicalDevice physicalDevice,
		const VkQueueFlagBits bits);

private:
	[[nodiscard]] static std::vector<VkQueueFamilyProperties> createQueueFamilyProperties(const VkPhysicalDevice& physicalDevice);
	
};

}

