#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

#include "struct_data.h"

namespace render
{
class CreatorImages
{
public:

	[[nodiscard]] static std::vector<VkImage> CreateImages(
		std::shared_ptr<DataDevice> device,
		std::shared_ptr<DataSwapchain> swapchain);
private:
};
}
