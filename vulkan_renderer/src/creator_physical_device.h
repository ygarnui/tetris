#pragma once

#include "struct_data.h"

#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

namespace render
{
class CreatorPhysicalDevice
{
public:

	[[nodiscard]] static std::vector<VkPhysicalDevice> CreatePhysicalDevices(std::shared_ptr<DataInstance> instanse);

	static void PrintInfo(const VkPhysicalDeviceProperties& physicalDevices);
};
}
