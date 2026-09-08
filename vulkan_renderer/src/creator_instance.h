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
	/*!
	\param[in] messengerCreateInfo chained in via pNext when not null; pass null to create the
	instance without the debug messenger (e.g. a release build with no validation layer to
	report anything).
	*/
	void static initInstanceCreateInfo(
		VkInstanceCreateInfo& InstanceCreateInfo,
		const VkApplicationInfo& applicationInfo,
		const VkDebugUtilsMessengerCreateInfoEXT* messengerCreateInfo,
		const std::vector<const char*>& extensions,
		const std::vector<const char*>& layers);
};
}
