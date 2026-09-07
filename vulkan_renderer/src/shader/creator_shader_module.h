#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include "../struct_data.h"


namespace render
{
class CreatorShaderModule
{
public:
	[[nodiscard]] static std::shared_ptr<DataShaderModule> CreateShaderModule(
		std::shared_ptr<DataDevice> device,
		const std::string& code);
};
}
