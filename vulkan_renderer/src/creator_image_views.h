#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

#include "struct_data.h"

namespace render
{
class CreatorImageView
{
public:

	[[nodiscard]] static std::vector<std::shared_ptr<DataImageView>> CreateImageViews(
		std::shared_ptr<DataDevice> device,
		const std::vector<VkImage>& images,
		const VkFormat format,
		const VkImageAspectFlags aspectFlags,
		const VkComponentMapping& components);

private:
};
}
