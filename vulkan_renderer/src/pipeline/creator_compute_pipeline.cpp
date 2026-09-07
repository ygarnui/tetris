#include "creator_compute_pipeline.h"

#include <logger_instance.h>

namespace render
{

std::shared_ptr<DataPipeline> CreatorComputePipeline::CreateComputePipeline(
	std::shared_ptr<DataDevice> logicalDevice,
	std::shared_ptr<DataPipelineLayout> pipelineLayout,
	const VkPipelineShaderStageCreateInfo& pipelineShaderStageCreateInfo)
{
	std::shared_ptr<DataPipeline> pipeline(
		new DataPipeline{
			{},
			logicalDevice
		},
		[](DataPipeline* p)
		{
			vkDestroyPipeline(p->device->device, p->pipeline, nullptr);
			delete p;
		}
	);

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.layout = pipelineLayout->pipline_layout;
	computePipelineCreateInfo.stage = pipelineShaderStageCreateInfo;

	const VkResult result = vkCreateComputePipelines(logicalDevice->device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline->pipeline);

	if (result != VK_SUCCESS) {
		LOGEXC(std::runtime_error, "[CreatorComputePipeline::CreateComputePipeline] failed to create graphics pipeline!");
	}

	return pipeline;
}

}
