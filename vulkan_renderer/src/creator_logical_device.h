#pragma once

#include "device_property.h"
#include "struct_data.h"

#include <vector>
#include <memory>

#include <vulkan/vulkan_core.h>

namespace render
{

class CreatorLogicalDevice
{
public:
	[[nodiscard]] static std::shared_ptr<DataDevice> CreateLogicalDevice(
		VkPhysicalDevice physicalDevice, 
		const std::vector<QueueFamilyIndices>& queueFamilyIndex,
		const VkPhysicalDeviceFeatures& physicalDeviceFeatures,
		const std::vector<const char*>& deviceExtensions);
private:
	static void initDeviceQueueCreateInfo(
		VkDeviceQueueCreateInfo& queueCreateInfo, 
		const uint32_t index, 
		float& queuePriority);

	static void initDeviceCreateInfo(
		VkDeviceCreateInfo& deviceCreateInfo,
		const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfo,
		const VkPhysicalDeviceFeatures& deviceFeatures,
		const std::vector<const char*>& deviceExtensions);
	

	//std::vector<VkDeviceQueueCreateInfo> device_queue_create_info_;
	VkDevice device_;
};
}