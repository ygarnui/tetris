#include "creator_fence.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{
	std::shared_ptr<DataFence> CreatorFence::CreateFence(std::shared_ptr<DataDevice> device)
	{
		std::shared_ptr<DataFence> fence(
			new DataFence{ 
				{},
				device,
			},
			[](DataFence* p)
			{
				vkDestroyFence(p->device->device, p->fence, nullptr);
				delete p;
			}
		);

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		auto res = vkCreateFence(device->device, &fenceInfo, nullptr, &fence->fence);

		if (res != VK_SUCCESS)
		{
			LOGEXC(std::runtime_error, "[CreatorFence::CreateFence] failed to create fence");
		}

		return fence;
	}
}
