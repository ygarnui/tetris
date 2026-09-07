#pragma once

#include "device_property.h"
#include "struct_data.h"

#include <vulkan/vulkan_core.h>

#include <memory>

namespace render
{
class CreatorQueueDescription
{
public:

	[[nodiscard]] static std::vector<VkQueue> CreateQueueDescription(
		std::shared_ptr<DataDevice> device,
		const std::vector<QueueFamilyIndices>& index);
};
}
