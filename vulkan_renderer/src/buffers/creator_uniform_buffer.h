#pragma once

#include "../struct_data.h"

namespace render
{
	class CreatorUniformBuffer
	{
	public:
		[[nodiscard]] static std::shared_ptr<DataUniformBuffer> CreateUniformBuffer(
			const uint64_t size,
			const VkBufferUsageFlags usage,
			std::shared_ptr<DataDevice> device,
			VkPhysicalDevice physical_device);
	};
}
