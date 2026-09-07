#include "creator_descriptor_set_layout.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{
	std::shared_ptr<DataDescriptorSetLayout> CreatorDescriptorSetLayout::CreateDescriptorSetLayout(
		const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings,
		std::shared_ptr<DataDevice> device)
	{
		std::shared_ptr<DataDescriptorSetLayout> dataLayout(
			new DataDescriptorSetLayout{ {}, device },
			[](DataDescriptorSetLayout* p)
			{
				vkDestroyDescriptorSetLayout(p->device->device, p->descriptor_set_layout, nullptr);
				delete p;
			}
		);
		 
		VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
		layoutCreateInfo.pBindings = layoutBindings.data();

		const VkResult result = vkCreateDescriptorSetLayout(
			device->device,
			&layoutCreateInfo,
			nullptr,
			&dataLayout->descriptor_set_layout);
		if (result != VK_SUCCESS)
		{
			LOGEXC(std::runtime_error, "[CreatorDescriptorSetLayout::CreateDescriptorSetLayout] Failed to create descriptor set layout");
		}

		return dataLayout;
	}
}