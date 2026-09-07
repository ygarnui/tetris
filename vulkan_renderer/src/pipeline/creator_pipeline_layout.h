#pragma once

#include "../struct_data.h"


namespace render
{
class CreatorPipelineLayout
{
public:
	[[nodiscard]] static std::shared_ptr<DataPipelineLayout> CreatePipelineLayout(
		std::shared_ptr<DataDevice> device,
		const std::vector<std::shared_ptr<DataDescriptorSetLayout>>& descriptorSetLayouts);
};
}
