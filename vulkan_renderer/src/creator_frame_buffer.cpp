#include "creator_frame_buffer.h"

#include <logger_instance.h>

namespace render
{
std::shared_ptr<DataFrameBuffer> CreatorFrameBuffer::CreateFrameBuffer(
	const std::vector<VkImageView>& imageViews,
	std::shared_ptr<DataRenderPass> renderPass,
	std::shared_ptr<DataDevice> device,
	const VkExtent2D& extent)
{
	std::shared_ptr<DataFrameBuffer> frameBuffer(
		new DataFrameBuffer{ 
			{},
			device,
			extent
		},
		[](DataFrameBuffer* p) 
		{
			vkDestroyFramebuffer(p->device->device, p->framebuffer, nullptr);
			delete p;
		}
	);

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass->render_pass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
	framebufferInfo.pAttachments = imageViews.data();
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;

	const VkResult result = vkCreateFramebuffer(device->device, &framebufferInfo, nullptr, &frameBuffer->framebuffer);

	if (result != VK_SUCCESS)
	{
        LOGEXC(std::runtime_error, "Failed to create framebuffer!");
	}

	return frameBuffer;
}
}
