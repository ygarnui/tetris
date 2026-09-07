#include "creator_image_views.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{

std::vector<std::shared_ptr<DataImageView>> CreatorImageView::CreateImageViews(
	std::shared_ptr<DataDevice> device,
	const std::vector<VkImage>& images,
	const VkFormat format,
	const VkImageAspectFlags aspectFlags,
	const VkComponentMapping& components)
{
	std::vector<std::shared_ptr<DataImageView>> imageViews(images.size());
	for (size_t imageViewIndex = 0; imageViewIndex < imageViews.size(); imageViewIndex++)
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = images[imageViewIndex];

		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = format;

		imageViewCreateInfo.components = components;

		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		std::shared_ptr<DataImageView> imageView(
			new DataImageView{ 
				{},
				device 
			},
			[](DataImageView* p) {
				vkDestroyImageView(p->device->device, p->image_view, nullptr);
				delete p;
			}
		);

		const VkResult result = vkCreateImageView(device->device, &imageViewCreateInfo, nullptr, &imageView->image_view);
		if (result != VK_SUCCESS)
		{
			LOGEXC(std::runtime_error, "[CreatorImageView::CreateImageViews] Failed to create image view!");
		}

		imageViews[imageViewIndex] = imageView;
	}

	return imageViews;
}

}
