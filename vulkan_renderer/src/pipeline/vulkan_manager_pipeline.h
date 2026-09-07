#pragma once

#include "../shader/vulkan_manager_shader_program.h"
#include "../manager_device.h"
#include "../vulkan_manager_render_pass.h"
#include "../struct_data.h"
#include "../src/manager_window.h"
#include "../manager_base.h"

#include <manager_pipeline.h>
#include <render_id.h>
#include <vector.h>
#include <vulkan/vulkan.h>

#include <memory>


namespace render
{
	struct DetailPipeline
	{
		struct DetailGraphics
		{
			RenderId render_pass_id;
			VkPrimitiveTopology topology;
			VkPolygonMode polygon_mode;
		};

		RenderId shader_program_id;
		RenderId logical_device_id;
		VkPipelineBindPoint type;
		std::optional<DetailGraphics> detail_graphics;
	};

class VulkanManagerPipeline : public ManagerPipeline, public ManagerBase
{
public:
	static std::shared_ptr<VulkanManagerPipeline>& Get();

	~VulkanManagerPipeline();

	VulkanManagerPipeline(const VulkanManagerPipeline&) = delete;
	VulkanManagerPipeline(VulkanManagerPipeline&&) = delete;

	VulkanManagerPipeline& operator= (const VulkanManagerPipeline&) = delete;
	VulkanManagerPipeline& operator= (VulkanManagerPipeline&&) = delete;

	[[nodiscard]] std::shared_ptr<DataPipeline> GetPipeline(const render::RenderId& pipelineId);
	[[nodiscard]] std::shared_ptr<DataPipelineLayout> GetPipelineLayoutData(const render::RenderId& pipelineId);

	[[nodiscard]] RenderId CreateGraphicsPipeline(
		const GraphicsWindowId& windowId,
		const ShaderProgramId& shaderProgramId,
		const RenderPassId& renderPass,
		const description::TypeTopology topology,
		const description::TypePolygonMode polygonMode,
		description::SampleCountFlagBits sampleCountFlagBits,
		bool depthWriteEnable,
		bool depthTestEnable,
		uint32_t sizePatch) override;

	[[nodiscard]] RenderId CreateComputePipeline(
		const GraphicsWindowId& windowId,
		const ShaderProgramId& shaderProgramId) override;

	void DeletePipeline(RenderId& pipelineId) override;

private:
	VulkanManagerPipeline();


	[[nodiscard]] render::RenderId createGraphicsPipeline(
		const ShaderProgramId& shaderProgramId,
		VkPrimitiveTopology topology,
		VkPolygonMode polygonMode,
		VkSampleCountFlagBits sampleCountFlagBits,
		const LogicalDeviceId& logicalDeviceId,
		const RenderPassId& renderPassId,
		const bool depthWriteEnable,
		const bool depthTestEnable,
		uint32_t sizePatch
	);

	[[nodiscard]] render::RenderId createComputePipeline(
		const ShaderProgramId& shaderProgramId,
		const LogicalDeviceId& logicalDeviceId);

	void deleteShaderProgramPipeline(render::RenderId& pipelineId);

	size_t next_pipeline_id_ = 0;

	Vector<std::shared_ptr<DataPipelineLayout>, render::RenderId> pipeline_layouts_;

	Vector<std::shared_ptr<DataPipeline>, render::RenderId> pipelines_;
	Vector<DetailPipeline, render::RenderId> detail_pipelines_;
};
}
