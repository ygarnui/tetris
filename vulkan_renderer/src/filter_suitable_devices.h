#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

#include "device_property.h"
#include "struct_data.h"

namespace render
{
class FilterSuitableDevices
{
public:
	
	[[nodiscard]] static std::vector<PhysicalDeviceProperties> LeaveOnlySuitable(
		const std::vector<VkPhysicalDevice>& physicalDevices, 
		std::shared_ptr<DataSurface> surface,
		const std::vector<const char*>& deviceExtensions,
		const DataSwapchainSupportDetails& swapChainSupportDetailsData);
	
	[[nodiscard]] static bool CheckPhysicalDevice(
		const VkPhysicalDevice device,
		std::shared_ptr<DataSurface> surface,
		const std::vector<const char*>& deviceExtensions,
		const DataSwapchainSupportDetails& swapChainSupportDetailsData,
		PhysicalDeviceProperties& deviceProperty);

private:
	
	[[nodiscard]] static bool isDeviceParamSuitable(const VkPhysicalDevice& device, VkPhysicalDeviceFeatures& deviceFeatures);

	[[nodiscard]] static bool isDeviceQueueSuitable(
		const VkPhysicalDevice device, 
		std::shared_ptr<DataSurface> surface, 
		std::vector<QueueFamilyIndices>& indices);

	[[nodiscard]] static bool checkDeviceExtensionSupport(const VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
	[[nodiscard]] static bool chekSwapchainSupportDetails(const DataSwapchainSupportDetails& swapChainSupportDetails);
	[[nodiscard]] static bool checkDeviceParam(const VkPhysicalDeviceProperties& deviceProperties, const VkPhysicalDeviceFeatures& deviceFeatures);
};
}
