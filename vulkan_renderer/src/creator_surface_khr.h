#pragma once

#include "struct_data.h"

#include <vulkan/vulkan_core.h>

#include <memory>
#include <functional>

namespace render
{
class CreatorSurfaceKHR
{
public:
	[[nodiscard]] static std::shared_ptr<DataSurface> CreateSurfaceKHR(
		std::shared_ptr<DataInstance> instance, 
		const std::function<DataResult(std::shared_ptr<DataInstance>, std::shared_ptr<DataSurface>)>& createWindowSurface,
		bool externalDeleter);
};
}
