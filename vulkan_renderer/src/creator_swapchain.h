#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include "struct_data.h"
#include "device_property.h"


namespace render
{
class CreatorSwapchain
{
public:
	[[nodiscard]] static std::shared_ptr<DataSwapchain> CreateSwapchain(
		const DataSwapchainSupportDetails& swapChainSupport,
		std::shared_ptr<DataDevice> device,
		VkPhysicalDevice physicalDevice,
		std::shared_ptr<DataSurface> surface,
		const VkExtent2D& extent,
		const QueueFamilyIndices& indices);

private:

	[[nodiscard]] static VkSwapchainCreateInfoKHR createSwapchainCreateInfoKHR(
		std::shared_ptr<DataSurface> surface,
		VkSurfaceCapabilitiesKHR capabilities,
		VkSurfaceFormatKHR surfaceFormat,
		const VkExtent2D& extent,
		const QueueFamilyIndices& indices,
		const VkPresentModeKHR presentMode);
};
}
