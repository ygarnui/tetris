#include "creator_descriptor_pool.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{

std::shared_ptr<DataDescriptorPool> CreatorDescriptorPool::CreateDescriptorPool(
	const uint32_t numUniforms,
	const uint32_t numSamplers,
	const uint32_t numSwapChainImages,
	std::shared_ptr<DataDevice> device)
{
	std::shared_ptr<DataDescriptorPool> pool(
		new DataDescriptorPool(
			{},
			device
			),
		[](DataDescriptorPool* p)
		{
			vkDestroyDescriptorPool(p->device->device, p->descriptor_pool, nullptr);
			delete p;
		}
	);
	
	const std::vector<VkDescriptorPoolSize> poolSizes =
	{
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32_t)numUniforms * numSwapChainImages },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, (uint32_t)numUniforms * numSwapChainImages },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)numSamplers * numSwapChainImages },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, (uint32_t)numSamplers * numSwapChainImages },
		// A ray tracing shader program binds one top level structure per swapchain image.
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, (uint32_t)numSwapChainImages }
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = (numUniforms + numSamplers) * numSwapChainImages;

	const VkResult result = vkCreateDescriptorPool(device->device, &poolInfo, nullptr, &pool->descriptor_pool);
	if (result != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorDescriptorPool::CreateDescriptorPool] failed to create descriptor pool!");
	}

	return pool;
}

}
