#include "creator_images.h"

#include <logger_instance.h>

namespace render
{

std::vector<VkImage> CreatorImages::CreateImages(
    std::shared_ptr<DataDevice> device,
    std::shared_ptr<DataSwapchain> swapchain)
{
    uint32_t imageCount = 0;
    auto res = vkGetSwapchainImagesKHR(device->device, swapchain->swapchain, &imageCount, nullptr);
    if (res != VK_SUCCESS)
    {
        LOGEXC(std::runtime_error, "[CreatorImages::CreateImages]");
    }

    std::vector<VkImage> images(imageCount);
    res = vkGetSwapchainImagesKHR(device->device, swapchain->swapchain, &imageCount, images.data());
    if (res != VK_SUCCESS)
    {
        LOGEXC(std::runtime_error, "[CreatorImages::CreateImages]");
    }

    return images;
}

}
