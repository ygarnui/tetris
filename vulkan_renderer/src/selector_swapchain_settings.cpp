#include "selector_swapchain_settings.h"
#include "required_device_parameters.h"

#include <logger_instance.h>

namespace render
{
bool SelectorSwapchainSettings::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats,
    const VkColorSpaceKHR& colorSpace,
    const VkFormat& format,
    VkSurfaceFormatKHR& findFormat)
{
    bool res = false;
    for (const auto& availableFormat : availableFormats) 
    {
        res = CheckType(availableFormat.format, format) && 
        	  CheckType(availableFormat.colorSpace, colorSpace);
        if (res) {
            findFormat = availableFormat;
            return true;
        }
    }
    findFormat = availableFormats[0];
    return false;
}

VkPresentModeKHR SelectorSwapchainSettings::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    // Only VK_PRESENT_MODE_FIFO_KHR mode is guaranteed to be available

    for (const auto& availablePresentMode : availablePresentModes) {
        if (CheckType(availablePresentMode, VK_PRESENT_MODE_MAILBOX_KHR))
        {
            return availablePresentMode;
        }
    }

    LOG(Loglvl::info, "[SelectorSwapchainSettings::ChooseSwapPresentMode] the surface of the current device don't have VK_PRESENT_MODE_MAILBOX_KHR");

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SelectorSwapchainSettings::ChooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    const VkExtent2D& extent)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    else {
        VkExtent2D actualExtent = {};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));

        return actualExtent;
    }
}

}
