#include "vulkan_manager_pipeline.h"

#include "creator_pipeline_layout.h"
#include "creator_graphics_pipeline.h"
#include "creator_compute_pipeline.h"
#include "../src/converter_description.h"

#include <guard_next_id.h>

namespace render
{
	VulkanManagerPipeline::VulkanManagerPipeline()
	{

	}

	std::shared_ptr<VulkanManagerPipeline>& VulkanManagerPipeline::Get()
	{
		static std::shared_ptr<VulkanManagerPipeline> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<VulkanManagerPipeline>(new VulkanManagerPipeline());
		}
		return manager;
	}

	VulkanManagerPipeline::~VulkanManagerPipeline()
	{
		LOG(Loglvl::debug, "[VulkanManagerPipeline::~VulkanManagerPipeline]");
	}

	std::shared_ptr<DataPipeline> VulkanManagerPipeline::GetPipeline(const RenderId& pipelineId)
	{
		return pipelines_[pipelineId];
	}

	std::shared_ptr<DataPipelineLayout> VulkanManagerPipeline::GetPipelineLayoutData(const RenderId& pipelineId)
	{
		return pipeline_layouts_[pipelineId];
	}

	RenderId VulkanManagerPipeline::CreateGraphicsPipeline(
		const GraphicsWindowId& windowId,
		const ShaderProgramId& shaderProgramId,
		const RenderPassId& renderPass,
		const description::TypeTopology topology,
		const description::TypePolygonMode polygonMode,
		description::SampleCountFlagBits sampleCountFlagBits,
		bool depthWriteEnable,
		bool depthTestEnable,
		uint32_t sizePatch)
	{
		return createGraphicsPipeline(
			shaderProgramId,
			ConverterDescription::TypeTopologyToPrimitiveTopology(topology),
			ConverterDescription::TypePolygonModeToVkPolygonMode(polygonMode),
			ConverterDescription::ConvertAttachmentSamples(sampleCountFlagBits),
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			renderPass,
			depthWriteEnable,
			depthTestEnable,
			sizePatch
		);
	}

	RenderId VulkanManagerPipeline::CreateComputePipeline(const GraphicsWindowId& windowId,
		const ShaderProgramId& shaderProgramId)
	{
		return createComputePipeline(
			shaderProgramId,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId));
	}

	void VulkanManagerPipeline::DeletePipeline(RenderId& pipelineId)
	{
		deleteShaderProgramPipeline(pipelineId);
	}

	RenderId VulkanManagerPipeline::createGraphicsPipeline(
		const ShaderProgramId& shaderProgramId,
		VkPrimitiveTopology topology,
		VkPolygonMode polygonMode,
		VkSampleCountFlagBits sampleCountFlagBits,
		const LogicalDeviceId& logicalDeviceId,
		const RenderPassId& renderPassId,
		const bool depthWriteEnable,
		const bool depthTestEnable,
		uint32_t sizePatch
	)
	{
		size_t numExistPipeline = 0;
		for (const auto& detailPipeline : detail_pipelines_)
		{
			if (detailPipeline.detail_graphics &&
				detailPipeline.logical_device_id == logicalDeviceId &&
				detailPipeline.shader_program_id == shaderProgramId)
			{
				if (detailPipeline.detail_graphics->render_pass_id == renderPassId &&
					detailPipeline.detail_graphics->topology == topology &&
					detailPipeline.detail_graphics->polygon_mode == polygonMode)
				{
					return GeneratorId::GenerateId(numExistPipeline);
				}
			}
			numExistPipeline++;
		}

		auto nextPipelineId = GeneratorId::GenerateUniqueId<RenderId>(next_pipeline_id_);

		auto guardResize = utils::GuardResize::MayBeResize(
			next_pipeline_id_,
			[](const std::shared_ptr<DataPipeline>& pipeline) { return bool(!pipeline); },
			pipelines_,
			pipeline_layouts_,
			detail_pipelines_);

		DetailPipeline::DetailGraphics detailGraphics{};
		detailGraphics.render_pass_id = renderPassId;
		detailGraphics.polygon_mode = polygonMode;
		detailGraphics.topology = topology;

		detail_pipelines_[nextPipelineId] = { shaderProgramId, logicalDeviceId, VK_PIPELINE_BIND_POINT_GRAPHICS, detailGraphics };

		const auto pipelineShaderStageCreateInfo = VulkanManagerShaderProgram::Get()->CreatePipelineShaderStageCreateInfo(shaderProgramId);
		const auto& vertexInputAttributeDescription = VulkanManagerShaderProgram::Get()->GetVertexInputAttributeDescriptions(shaderProgramId);
		const auto& vertexInputBindingDescription = VulkanManagerShaderProgram::Get()->GetVertexInputBindingDescriptions(shaderProgramId);

		const auto& descriptorSetLayoutsData = VulkanManagerShaderProgram::Get()->GetDescriptorSetLayoutsData(shaderProgramId);

		pipeline_layouts_[nextPipelineId] = CreatorPipelineLayout::CreatePipelineLayout(
				ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
				descriptorSetLayoutsData);

		pipelines_[nextPipelineId] = CreatorGraphicsPipeline::CreateGraphicsPipeline(
			pipeline_layouts_[nextPipelineId],
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			VulkanManagerRenderPass::Get()->GetRenderPass(renderPassId),
			topology,
			polygonMode,
			sampleCountFlagBits,
			pipelineShaderStageCreateInfo,
			vertexInputBindingDescription,
			vertexInputAttributeDescription,
			depthWriteEnable,
			depthTestEnable,
			sizePatch
		);

		return nextPipelineId;
	}

	RenderId VulkanManagerPipeline::createComputePipeline(const ShaderProgramId& shaderProgramId, const LogicalDeviceId& logicalDeviceId)
	{
		for (const auto& detailPipeline : detail_pipelines_)
		{
			if (detailPipeline.detail_graphics &&
				detailPipeline.logical_device_id == logicalDeviceId &&
				detailPipeline.shader_program_id == shaderProgramId)
			{
					return shaderProgramId;
			}
		}

		auto nextPipelineId = GeneratorId::GenerateUniqueId<RenderId>(next_pipeline_id_);

		auto guardResize = utils::GuardResize::MayBeResize(
			next_pipeline_id_,
			[](const std::shared_ptr<DataPipeline>& pipeline) { return bool(!pipeline); },
			pipelines_,
			pipeline_layouts_,
			detail_pipelines_);

		detail_pipelines_[nextPipelineId] = { shaderProgramId, logicalDeviceId, VK_PIPELINE_BIND_POINT_COMPUTE, std::nullopt };

		const auto pipelineShaderStageCreateInfo = VulkanManagerShaderProgram::Get()->CreatePipelineShaderStageCreateInfo(shaderProgramId).back();

		const auto& descriptorSetLayoutsData = VulkanManagerShaderProgram::Get()->GetDescriptorSetLayoutsData(shaderProgramId);

		pipeline_layouts_[nextPipelineId] = CreatorPipelineLayout::CreatePipelineLayout(
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			descriptorSetLayoutsData);

		pipelines_[nextPipelineId] = CreatorComputePipeline::CreateComputePipeline(
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			pipeline_layouts_[nextPipelineId],
			pipelineShaderStageCreateInfo);

		return nextPipelineId;
	}

	void VulkanManagerPipeline::deleteShaderProgramPipeline(render::RenderId& pipelineId)
	{
		next_pipeline_id_ = std::min(next_pipeline_id_, pipelineId.GetId());
		pipelines_[pipelineId] = nullptr;
		pipeline_layouts_[pipelineId] = nullptr;
		detail_pipelines_[pipelineId] = {};
		pipelineId = GeneratorId::GenerateInvalidId();
	}
}
