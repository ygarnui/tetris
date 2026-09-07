#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace render
{
class SelectorSwapchainSettings
{
public:
	// an empty vector cannot come here
	// if this has happened, then it is necessary to prevent such behavior
	[[nodiscard]] static bool ChooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& availableFormats,
		const VkColorSpaceKHR& colorSpace,
		const VkFormat& format,
		VkSurfaceFormatKHR& find);

	// an empty vector cannot come here
	// if this has happened, then it is necessary to prevent such behavior
	[[nodiscard]] static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	[[nodiscard]] static VkExtent2D ChooseSwapExtent(
		const VkSurfaceCapabilitiesKHR& capabilities, 
		const VkExtent2D& extent);

private:

};
}
