#pragma once

#include "../struct_data.h"

namespace render
{
	class CreatorDescriptorSetLayout
	{
	public:
		[[nodiscard]] static std::shared_ptr<DataDescriptorSetLayout> CreateDescriptorSetLayout(
			const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings,
			std::shared_ptr<DataDevice> device);
	};
}
