#include "creator_queue_descriptor.h"
#include <vector>

namespace render
{
	std::vector<VkQueue> CreatorQueueDescription::CreateQueueDescription(
		std::shared_ptr<DataDevice> device,
		const std::vector<QueueFamilyIndices>& index)
	{
		std::vector<VkQueue> queue(index.size());
		for (size_t i = 0; i < index.size(); i++)
		{
			vkGetDeviceQueue(device->device, static_cast<uint32_t>(index[i].getIndex().value()), static_cast<uint32_t>(i), &queue[i]);
		}
		
		return queue;
	}
}
