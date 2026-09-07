#include "creator_graphics_pipeline.h"
#include "../shader/creator_shader_module.h"
#include "../shader/reader_shader.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{

std::shared_ptr<DataPipeline> CreatorGraphicsPipeline::CreateGraphicsPipeline(
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
	uint32_t sizePatch)
	{
	auto pipelineInputAssemblyStateCreateInfo = createPipelineInputAssemblyStateCreateInfo(topology);

	std::vector<VkViewport> viewPorts;
	std::vector<VkRect2D> scissors;

	// With dynamic state it's even possible to specify different viewports and or scissor rectangles within a single command buffer.
	// Without dynamic state, the viewportand scissor rectangle need to be set in the pipeline using the VkPipelineViewportStateCreateInfo 
	// struct.This makes the viewportand scissor rectangle for this pipeline immutable.
	// Any changes required to these values would require a new pipeline to be created with the new values.
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkViewport viewport;
	viewPorts.push_back(viewport);

	VkRect2D rect2D;
	scissors.push_back(rect2D);

	auto pipelineVertexInputStateCreateInfo = createPipelineVertexInputStateCreateInfo(
		bindingDescription,
		arrtDesc);

	auto pipelineViewportStateCreateInfo = createPipelineViewportStateCreateInfo(viewPorts, scissors);

	auto pipelineRasterizationStateCreateInfo = createPipelineRasterizationStateCreateInfo(polygonMode);

	auto pipelineMultisampleStateCreateInfo = createPipelineMultisampleStateCreateInfo(sampleCountFlagBits);

	auto pipelineDepthStencilStateCreateInfo = createPipelineDepthStencilStateCreateInfo(depthTestEnable, depthWriteEnable, VK_COMPARE_OP_LESS);

	auto pipelineColorBlendAttachmentState = createPipelineColorBlendAttachmentState();
	std::vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates;
	pipelineColorBlendAttachmentStates.push_back(pipelineColorBlendAttachmentState);
	auto pipelineColorBlendStateCreateInfo = createPipelineColorBlendStateCreateInfo(pipelineColorBlendAttachmentStates);

	auto pipelineDynamicStateCreateInfo = createPipelineDynamicStateCreateInfo(dynamicStates);

	VkPipelineTessellationStateCreateInfo* ptrPipelineTesselationStateCreateInfo = nullptr;
	VkPipelineTessellationStateCreateInfo pipelineTesselationStateCreateInfo;
	if (sizePatch > 0)
	{
		pipelineTesselationStateCreateInfo = createPipelineTessellationStateCreateInfo(sizePatch);
		ptrPipelineTesselationStateCreateInfo = &pipelineTesselationStateCreateInfo;
	}

	auto graphicsPipelineCreateInfo = createGraphicsPipelineCreateInfo(
		pipelineShaderStageCreateInfo,
		&pipelineVertexInputStateCreateInfo,
		&pipelineInputAssemblyStateCreateInfo,
		&pipelineViewportStateCreateInfo,
		&pipelineRasterizationStateCreateInfo,
		&pipelineMultisampleStateCreateInfo,
		&pipelineDepthStencilStateCreateInfo,
		&pipelineColorBlendStateCreateInfo,
		&pipelineDynamicStateCreateInfo,
		ptrPipelineTesselationStateCreateInfo,
		pipelineLayout,
		renderPass,
		nullptr);

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

	auto res = vkCreateGraphicsPipelines(logicalDevice->device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline->pipeline);

	if (res != VK_SUCCESS) {
		LOGEXC(std::runtime_error, "[CreatorGraphicsPipeline::CreateGraphicsPipeline] failed to create graphics pipeline!");
	}

	return pipeline;
}

VkViewport CreatorGraphicsPipeline::CreateViewport(const VkExtent2D& extent)
{
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = static_cast<float>(extent.height);
	viewport.width = static_cast<float>(extent.width);
	viewport.height = -static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	return viewport;
}

VkRect2D CreatorGraphicsPipeline::CreateRect2D(const VkExtent2D& extent)
{
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;

	return scissor;
}

VkPipelineShaderStageCreateInfo CreatorGraphicsPipeline::createPipelineShaderStageCreateInfo(
	const VkShaderStageFlagBits shaderStage,
	const VkShaderModule module,
	const std::string& funcName)
{
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = shaderStage;
	fragShaderStageInfo.module = module;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr;

	return fragShaderStageInfo;
}

VkPipelineVertexInputStateCreateInfo CreatorGraphicsPipeline::createPipelineVertexInputStateCreateInfo(
	const std::vector<VkVertexInputBindingDescription>& bind_desc,
	const std::vector<VkVertexInputAttributeDescription>& arrtr_desc)
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bind_desc.size());
	vertexInputInfo.pVertexBindingDescriptions = bind_desc.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(arrtr_desc.size());
	vertexInputInfo.pVertexAttributeDescriptions = arrtr_desc.data();

	return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo CreatorGraphicsPipeline::createPipelineInputAssemblyStateCreateInfo(const VkPrimitiveTopology topology)
{
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = topology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	return inputAssembly;
}

VkPipelineViewportStateCreateInfo CreatorGraphicsPipeline::createPipelineViewportStateCreateInfo(
	const std::vector<VkViewport>& viewPort,
	const std::vector<VkRect2D>& scissor)
{
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = static_cast<uint32_t>(viewPort.size());
	viewportState.pViewports = viewPort.data();
	viewportState.scissorCount = static_cast<uint32_t>(scissor.size());
	viewportState.pScissors = scissor.data();

	return viewportState;
}

VkPipelineRasterizationStateCreateInfo CreatorGraphicsPipeline::createPipelineRasterizationStateCreateInfo(const VkPolygonMode polygonMode)
{
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_TRUE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = polygonMode;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	return rasterizer;
}

VkPipelineMultisampleStateCreateInfo CreatorGraphicsPipeline::createPipelineMultisampleStateCreateInfo(VkSampleCountFlagBits sampleCountFlagBits)
{
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = sampleCountFlagBits;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	return multisampling;
}

VkPipelineDepthStencilStateCreateInfo CreatorGraphicsPipeline::createPipelineDepthStencilStateCreateInfo(
	VkBool32 depthTestEnable,
	VkBool32 depthWriteEnable,
	VkCompareOp depthCompareOp)
{
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = depthTestEnable;
	depthStencil.depthWriteEnable = depthWriteEnable;

	depthStencil.depthCompareOp = depthCompareOp;

	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional

	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	return depthStencil;
}

VkPipelineColorBlendAttachmentState CreatorGraphicsPipeline::createPipelineColorBlendAttachmentState()
{
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	return colorBlendAttachment;
}

VkPipelineColorBlendStateCreateInfo CreatorGraphicsPipeline::createPipelineColorBlendStateCreateInfo(const std::vector<VkPipelineColorBlendAttachmentState>& vec)
{
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = static_cast<uint32_t>(vec.size());
	colorBlending.pAttachments = vec.data();
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	return colorBlending;
}

VkPipelineDynamicStateCreateInfo CreatorGraphicsPipeline::createPipelineDynamicStateCreateInfo(std::vector<VkDynamicState>& dynamicStates)
{
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	return dynamicState;
}

VkPipelineTessellationStateCreateInfo CreatorGraphicsPipeline::createPipelineTessellationStateCreateInfo(uint32_t sizePatch)
{
	VkPipelineTessellationStateCreateInfo tessellationState{};
	tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tessellationState.pNext = nullptr;
	tessellationState.flags = 0;
	tessellationState.patchControlPoints = sizePatch; // КРИТИЧНО: должно быть 4 (layout(vertices = 4

	return tessellationState;
}

VkGraphicsPipelineCreateInfo CreatorGraphicsPipeline::createGraphicsPipelineCreateInfo(
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
	std::shared_ptr<DataPipeline> basePipeline)
{
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(pipelineShaderStageCreateInfo.size());
	pipelineInfo.pStages = pipelineShaderStageCreateInfo.data();

	pipelineInfo.pVertexInputState = pipelineVertexInputStateCreateInfo;
	pipelineInfo.pInputAssemblyState = pipelineInputAssemblyStateCreateInfo;
	pipelineInfo.pViewportState = pipelineViewportStateCreateInfo;
	pipelineInfo.pRasterizationState = pipelineRasterizationStateCreateInfo;
	pipelineInfo.pMultisampleState = pipelineMultisampleStateCreateInfo;
	pipelineInfo.pDepthStencilState = pipelineDepthStencilStateCreateInfo; // Optional
	pipelineInfo.pColorBlendState = pipelineColorBlendStateCreateInfo;
	pipelineInfo.pDynamicState = pipelineDynamicStateCreateInfo; // Optional
	pipelineInfo.pTessellationState = pipelineTessellationStateCreateInfo;

	pipelineInfo.layout = pipelineLayout->pipline_layout;

	pipelineInfo.renderPass = renderPass->render_pass;
	pipelineInfo.subpass = 0;

	if (basePipeline)
	{
		pipelineInfo.basePipelineHandle = basePipeline->pipeline;
	}
	else
	{
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	}
	pipelineInfo.basePipelineIndex = -1; // Optional

	return pipelineInfo;
}

}
