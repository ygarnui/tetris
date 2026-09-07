#include "creator_swapchain_support_details.h"

#include <logger_instance.h>
#include <vulkan/vulkan_core.h>

#include <stdexcept>

namespace render
{
DataSwapchainSupportDetails CreatorSwapchainSupportDetails::CreateSwapchainSupportDetails(
	VkPhysicalDevice device,
	VkSurfaceKHR surface)
{
	DataSwapchainSupportDetails details{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount == 0)
	{
		LOGEXC(std::runtime_error, "[CreatorSwapchainSupportDetails::CreateSwapchainSupportDetails] Physical device format count for surface is 0!");
	}

	details.formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount == 0)
	{
		LOGEXC(std::runtime_error, "[CreatorSwapchainSupportDetails::CreateSwapchainSupportDetails] Physical device present mode count for surface is 0!");		
	}

	details.present_modes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.present_modes.data());

	return details;
}
}