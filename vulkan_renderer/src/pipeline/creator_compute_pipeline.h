#pragma once

#include "../struct_data.h"

#include <memory>

#include <vulkan/vulkan.h>

namespace render
{
	class CreatorComputePipeline
	{
	public:

		[[nodiscard]] static std::shared_ptr<DataPipeline> CreateComputePipeline(
			std::shared_ptr<DataDevice> logicalDevice,
			std::shared_ptr<DataPipelineLayout> pipelineLayout,
			const VkPipelineShaderStageCreateInfo& pipelineShaderStageCreateInfo);
	};
}
