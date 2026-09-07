#pragma once

#include "struct_data.h"

namespace render
{
	class CreatorSwapchainSupportDetails
	{
	public:
		[[nodiscard]] static DataSwapchainSupportDetails CreateSwapchainSupportDetails(
			VkPhysicalDevice device,
			VkSurfaceKHR surface);
	};
}
