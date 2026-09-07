#include "creator_command_pool.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{
std::shared_ptr<DataCommandPool> CreatorCommandPool::CreateCommandPool(
	std::shared_ptr<DataDevice> device,
	QueueFamilyIndices queueFamilyIndices)
{
	std::shared_ptr<DataCommandPool> commandPool(
		new DataCommandPool{ 
			{},
			device,
		},
		[](DataCommandPool* p) 
		{
			vkDestroyCommandPool(p->device->device, p->command_pool, nullptr);
			delete p;
		}
	);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

	auto res = vkCreateCommandPool(device->device, &poolInfo, nullptr, &commandPool->command_pool);

	if (res != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorCommandPool::CreateCommandPool] failed to create command pool");
	}

	res = vkResetCommandPool(device->device, commandPool->command_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	if (res != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorCommandPool::CreateCommandPool] failed to reset command pool");
	}

	return commandPool;
}
}
