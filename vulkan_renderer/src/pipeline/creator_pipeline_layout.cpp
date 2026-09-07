#include "creator_pipeline_layout.h"

#include <logger_instance.h>

#include <algorithm>
#include <stdexcept>

namespace render
{

std::shared_ptr<DataPipelineLayout> CreatorPipelineLayout::CreatePipelineLayout(
	std::shared_ptr<DataDevice> device,
	const std::vector<std::shared_ptr<DataDescriptorSetLayout>>& descriptorSetLayouts)
{
	std::shared_ptr<DataPipelineLayout> pipelineLayout(
		new DataPipelineLayout{
			{},
			device
		},
		[](DataPipelineLayout* p)
		{
			vkDestroyPipelineLayout(p->device->device, p->pipline_layout, nullptr);
			delete p;
		}
	);

	std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts(descriptorSetLayouts.size());
	std::transform(descriptorSetLayouts.cbegin(), descriptorSetLayouts.cend(), vkDescriptorSetLayouts.begin(),
		[](std::shared_ptr<DataDescriptorSetLayout> descriptorSetLayout)
		{
			return descriptorSetLayout->descriptor_set_layout;
		}
	);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = vkDescriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	const VkResult result = vkCreatePipelineLayout(device->device, &pipelineLayoutInfo, nullptr, &pipelineLayout->pipline_layout);
	if (result != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorPipelineLayout::CreatePipelineLayout] failed to create pipeline layout");
	}

	return pipelineLayout;
}

}
