#pragma once
#include "buffer_description.h"


namespace render
{
	class ManagerPipeline
	{
	public:
		[[nodiscard]] virtual RenderId CreateGraphicsPipeline(
			const GraphicsWindowId& windowId,
			const ShaderProgramId& shaderProgramId,
			const RenderPassId& renderPass,
			const description::TypeTopology topology,
			const description::TypePolygonMode polygonMode,
			description::SampleCountFlagBits sampleCountFlagBits,
			bool depthWriteEnable,
			bool depthTestEnable,
			uint32_t sizePatch
		) = 0;

		[[nodiscard]] virtual RenderId CreateComputePipeline(
			const GraphicsWindowId& windowId,
			const ShaderProgramId& shaderProgramId) = 0;

		virtual void DeletePipeline(RenderId& pipelineId) = 0;
	};
}
