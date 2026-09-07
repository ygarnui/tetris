#include "creator_semaphore.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{

std::shared_ptr<DataSemaphore> CreatorSemaphore::CreateSemaphore(std::shared_ptr<DataDevice> device)
{
	std::shared_ptr<DataSemaphore> semaphore(
		new DataSemaphore{ {}, device },
		[](DataSemaphore* p)
		{
			vkDestroySemaphore(p->device->device, p->semaphore, nullptr);
			delete p;
		}
	);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	auto res = vkCreateSemaphore(device->device, &semaphoreInfo, nullptr, &semaphore->semaphore);

	if (res != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorSemaphore::CreateSemaphore] failed to create semaphores");
	}

	return semaphore;
}
}
