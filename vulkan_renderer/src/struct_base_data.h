#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>

namespace render
{

	struct DataResult
	{
		VkResult result;
	};

	struct DataInstance
	{
		VkInstance instance;
	};

	struct DataSurface
	{
		VkSurfaceKHR surface;
		std::shared_ptr<DataInstance> instance;
	};
}
