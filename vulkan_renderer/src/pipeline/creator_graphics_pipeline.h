#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

#include "../struct_data.h"

namespace render
{
class CreatorGraphicsPipeline
{
public:

	[[nodiscard]] static std::shared_ptr<DataPipeline> CreateGraphicsPipeline(
		std::shared_ptr<DataPipelineLayout> pipelineLayout,
		std::shared_ptr<DataDevice> logicalDevice,
		std::shared_ptr<DataRenderPass> renderPass,
		VkPrimitiveTopology topology,
		VkPolygonMode polygonMode,
		VkSampleCountFlagBits sampleCountFlagBits,
		const std::vector<VkPipelineShaderStageCreateInfo>& pipelineShaderStageCreateInfo,
		const std::vector<VkVertexInputBindingDescription>& bindingDescription,
		const std::vector<VkVertexInputAttributeDescription>& arrtDesc,
		const bool depthWriteEnable,
		const bool depthTestEnable,
		uint32_t sizePatch);

	[[nodiscard]] static VkViewport CreateViewport(const VkExtent2D& extent);

	[[nodiscard]] static VkRect2D CreateRect2D(const VkExtent2D& extent);

private:
	[[nodiscard]] static VkPipelineShaderStageCreateInfo createPipelineShaderStageCreateInfo(
		const VkShaderStageFlagBits shaderStage,
		const VkShaderModule module,
		const std::string& funcName);

	[[nodiscard]] static VkPipelineVertexInputStateCreateInfo createPipelineVertexInputStateCreateInfo(
		const std::vector<VkVertexInputBindingDescription>& bind_desc,
		const std::vector<VkVertexInputAttributeDescription>& arrtr_desc);

	[[nodiscard]] static VkPipelineInputAssemblyStateCreateInfo createPipelineInputAssemblyStateCreateInfo(const VkPrimitiveTopology topology);

	[[nodiscard]] static VkPipelineViewportStateCreateInfo createPipelineViewportStateCreateInfo(
		const std::vector<VkViewport>& viewPort,
		const std::vector<VkRect2D>& scissor);

	[[nodiscard]] static VkPipelineRasterizationStateCreateInfo createPipelineRasterizationStateCreateInfo(const VkPolygonMode polygonMode);

	[[nodiscard]] static VkPipelineMultisampleStateCreateInfo createPipelineMultisampleStateCreateInfo(VkSampleCountFlagBits sampleCountFlagBits);

	[[nodiscard]] static VkPipelineDepthStencilStateCreateInfo createPipelineDepthStencilStateCreateInfo(
		VkBool32 depthTestEnable,
		VkBool32 depthWriteEnable,
		VkCompareOp depthCompareOp);

	[[nodiscard]] static VkPipelineColorBlendAttachmentState createPipelineColorBlendAttachmentState();

	[[nodiscard]] static VkPipelineColorBlendStateCreateInfo createPipelineColorBlendStateCreateInfo(const std::vector< VkPipelineColorBlendAttachmentState>& vec);

	[[nodiscard]] static VkPipelineDynamicStateCreateInfo createPipelineDynamicStateCreateInfo(std::vector<VkDynamicState>& dynamicStates);

	[[nodiscard]] static VkPipelineTessellationStateCreateInfo createPipelineTessellationStateCreateInfo(uint32_t sizePatch);

	[[nodiscard]] static VkGraphicsPipelineCreateInfo createGraphicsPipelineCreateInfo(
		const std::vector<VkPipelineShaderStageCreateInfo>& pipelineShaderStageCreateInfo,
		const VkPipelineVertexInputStateCreateInfo* pipelineVertexInputStateCreateInfo,
		const VkPipelineInputAssemblyStateCreateInfo* pipelineInputAssemblyStateCreateInfo,
		const VkPipelineViewportStateCreateInfo* pipelineViewportStateCreateInfo,
		const VkPipelineRasterizationStateCreateInfo* pipelineRasterizationStateCreateInfo,
		const VkPipelineMultisampleStateCreateInfo* pipelineMultisampleStateCreateInfo,
		const VkPipelineDepthStencilStateCreateInfo* pipelineDepthStencilStateCreateInfo,
		const VkPipelineColorBlendStateCreateInfo* pipelineColorBlendStateCreateInfo,
		const VkPipelineDynamicStateCreateInfo* pipelineDynamicStateCreateInfo,
		const VkPipelineTessellationStateCreateInfo* pipelineTessellationStateCreateInfo,
		std::shared_ptr<DataPipelineLayout> pipelineLayout,
		std::shared_ptr<DataRenderPass> renderPass,
		std::shared_ptr<DataPipeline> basePipeline);
};
}
