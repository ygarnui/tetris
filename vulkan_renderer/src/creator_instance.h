#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "struct_data.h"

namespace render
{

class CreatorInstance
{
public:

	[[nodiscard]] static std::shared_ptr<DataInstance> CreateInstace(const std::vector<const char*>& extensions, const std::vector<const char*>& layers);

private:
	void static initApplicationInfo(VkApplicationInfo& applicationInfo);
	void static initInstanceCreateInfo(
		VkInstanceCreateInfo& InstanceCreateInfo,
		const VkApplicationInfo& applicationInfo,
		VkDebugUtilsMessengerCreateInfoEXT& messengerCreateInfo,
		const std::vector<const char*>& extensions,
		const std::vector<const char*>& layers);
};
}
