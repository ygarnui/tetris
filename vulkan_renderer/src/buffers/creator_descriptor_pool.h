#pragma once

#include "../struct_data.h"

namespace render
{
class CreatorDescriptorPool
{
public:
	[[nodiscard]] static std::shared_ptr<DataDescriptorPool> CreateDescriptorPool(
		const uint32_t numUniforms,
		const uint32_t numSamplers,
		const uint32_t numSwapChainImages,
		std::shared_ptr<DataDevice> device);
};
}
