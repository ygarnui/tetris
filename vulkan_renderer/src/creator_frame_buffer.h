#pragma once

#include "struct_data.h"

namespace render
{
class CreatorFrameBuffer
{
public:
	[[nodiscard]] static std::shared_ptr<DataFrameBuffer> CreateFrameBuffer(
		const std::vector<VkImageView>& imageViews,
		std::shared_ptr<DataRenderPass> renderPass,
		std::shared_ptr<DataDevice> device,
		const VkExtent2D& extent);
};
}
