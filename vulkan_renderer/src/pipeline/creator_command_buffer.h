#pragma once

#include <vulkan/vulkan.h>

#include "../struct_data.h"

namespace render
{
class CreatorCommandBuffer
{
public:

	[[nodiscard]] static std::vector<VkCommandBuffer> CreateCommandBuffer(
		std::shared_ptr<DataCommandPool> commandPool,
		const VkExtent2D& extent,
		const size_t numFramebuffer);

	static void EndCommandBuffer(VkCommandBuffer commandBuffer);

	static void ResetCommandBuffer(VkCommandBuffer commandBuffer);

	static void RecreateCommandBuffer(
		VkCommandBuffer commandBuffer,
		std::shared_ptr<DataCommandPool> commandPool,
		const VkExtent2D& extent);
};
}
