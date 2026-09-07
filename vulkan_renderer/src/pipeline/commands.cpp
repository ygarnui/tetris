#include "commands.h"

#include "vulkan_manager_pipeline.h"
#include "../vulkan_manager_render_pass.h"
#include "../vulkan_manager_frame_buffer.h"
#include "../buffers/vulkan_manager_buffer.h"
#include "../buffers/vulkan_manager_uniform_buffer.h"
#include "../textures/vulkan_manager_textures.h"
#include "../raytracing/extension_functions.h"
#include "../raytracing/manager_ray_tracing.h"
#include "../manager_swapchain.h"
#include "creator_graphics_pipeline.h"

namespace render
{
	void Commands::AddCommand(std::shared_ptr<CmdBaseAbstact> cmd, QueueData& queueData)
	{
		switch (cmd->GetType())
		{
			case render::TypeCmdCall::BeginRenderPass:
				Commands::beginRenderPass(Commands::commandsConverter<CmdBeginRenderPass>(cmd), queueData);
				break;
			case render::TypeCmdCall::EndRenderPass:
				Commands::endRenderPass(Commands::commandsConverter<CmdEndRenderPass>(cmd), queueData);
				break;
			case render::TypeCmdCall::SetViewport:
				Commands::setViewport(Commands::commandsConverter<CmdSetViewport>(cmd), queueData);
				break;
			case render::TypeCmdCall::SetScissor:
				Commands::setScissor(Commands::commandsConverter<CmdSetScissor>(cmd), queueData);
				break;
			case render::TypeCmdCall::SetLineWidth:
				Commands::setLineWidth(Commands::commandsConverter<CmdSetLineWidth>(cmd), queueData);
				break;
			case render::TypeCmdCall::BindPipeline:
				Commands::bindPipeline(Commands::commandsConverter<CmdBindPipeline>(cmd), queueData);
				break;
			case render::TypeCmdCall::BindIndexBuffer:
				Commands::bindIndexBuffer(Commands::commandsConverter<CmdBindIndexBuffer>(cmd), queueData);
				break;
			case render::TypeCmdCall::BindVertexBuffers:
				Commands::bindVertexBuffers(Commands::commandsConverter<CmdBindVertexBuffers>(cmd), queueData);
				break;
			case render::TypeCmdCall::BindDescriptorSets:
				Commands::bindDescriptorSets(Commands::commandsConverter<CmdBindDescriptorSets>(cmd), queueData);
				break;
			case render::TypeCmdCall::Draw:
				Commands::draw(Commands::commandsConverter<CmdDraw>(cmd), queueData);
				break;
			case render::TypeCmdCall::DrawIndexed:
				Commands::drawIndexed(Commands::commandsConverter<CmdDrawIndexed>(cmd), queueData);
				break;
			case render::TypeCmdCall::DrawInderect:
				Commands::drawInderect(Commands::commandsConverter<CmdDrawInderect>(cmd), queueData);
				break;
			case render::TypeCmdCall::Dispatch:
				Commands::dispatch(Commands::commandsConverter<CmdDispatch>(cmd), queueData);
				break;
			case render::TypeCmdCall::TraceRays:
				Commands::traceRays(Commands::commandsConverter<CmdTraceRays>(cmd), queueData);
				break;
			case render::TypeCmdCall::ClearColorImage:
				Commands::clearColorImage(Commands::commandsConverter<CmdClearColorImage>(cmd), queueData);
				break;
			case render::TypeCmdCall::Test:
				Commands::test(Commands::commandsConverter<CmdTest>(cmd), queueData);
				break;
			case render::TypeCmdCall::Total:
				break;
			default:
				break;
		}
	}

	void Commands::beginRenderPass(std::shared_ptr<CmdBeginRenderPass> cmd, QueueData& queueData)
	{
		const auto& renderPassId = cmd->GetRenderPassId();
		const auto& frameBufferId = cmd->GetFrameBufferId();

		queueData.id_frame_buffer = frameBufferId;
		queueData.id_render_pass = renderPassId;

		const auto& renderPass = VulkanManagerRenderPass::Get()->GetRenderPass(renderPassId)->render_pass;
		const auto& frameBuffer = VulkanManagerFrameBuffer::Get()->GetFramebuffers(frameBufferId)[queueData.num_images];
		const auto& detail = VulkanManagerRenderPass::Get()->GetDetailRenderPass(renderPassId);
		
		std::vector<VkClearValue> clearValues;
		for (const auto& clearColor : detail.clearColors)
		{
			VkClearValue& clearValue = clearValues.emplace_back();
			clearValue.color = { {clearColor.r, clearColor.g, clearColor.b, clearColor.a} };
		}

		for (const auto& clearColor : detail.clearColorsResolve)
		{
			VkClearValue& clearValue = clearValues.emplace_back();
			clearValue.color = { {clearColor.r, clearColor.g, clearColor.b, clearColor.a} };
		}

		if (detail.clearDepth.has_value())
		{
			VkClearValue& clearValue = clearValues.emplace_back();
			clearValue.depthStencil.depth = detail.clearDepth->clear_depth;
			clearValue.depthStencil.stencil = detail.clearDepth->clear_stencil;
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frameBuffer->framebuffer;

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = frameBuffer->extent;

		vkCmdBeginRenderPass(queueData.command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void Commands::endRenderPass(std::shared_ptr<CmdEndRenderPass>, QueueData& queueData)
	{
		vkCmdEndRenderPass(queueData.command_buffer);
	}

	void Commands::setViewport(std::shared_ptr<CmdSetViewport> cmd, QueueData& queueData)
	{
		if (const auto& cmd_viewports = cmd->GetViewports(); !cmd_viewports.empty())
		{
			queueData.viewport = cmd_viewports.front();

			std::vector<VkViewport> viewports(cmd_viewports.size());
			for (size_t i = 0; i < viewports.size(); i++)
			{
				viewports[i] = VkViewport(
					cmd_viewports[i].x,
					cmd_viewports[i].y,
					cmd_viewports[i].width,
					cmd_viewports[i].height,
					cmd_viewports[i].minDepth,
					cmd_viewports[i].maxDepth);
			}

			vkCmdSetViewport(queueData.command_buffer, 0, (uint32_t)viewports.size(), viewports.data());
		}
		else
		{
			const auto& frameBuffer = VulkanManagerFrameBuffer::Get()->GetFramebuffers(cmd->GetFrameBufferId())[queueData.num_images];
			VkViewport viewport = CreatorGraphicsPipeline::CreateViewport(frameBuffer->extent);
			vkCmdSetViewport(queueData.command_buffer, 0, 1, &viewport);

			queueData.viewport.x = viewport.x;
			queueData.viewport.y = viewport.y;
			queueData.viewport.width = viewport.width;
			queueData.viewport.height = viewport.height;
			queueData.viewport.minDepth = viewport.minDepth;
			queueData.viewport.maxDepth = viewport.maxDepth;
		}
	}

	void Commands::setScissor(std::shared_ptr<CmdSetScissor> cmd, QueueData& queueData)
	{
		if (const auto& cmd_scissors = cmd->GetScissors(); !cmd_scissors.empty())
		{
			std::vector<VkRect2D> rects(cmd_scissors.size());
			for (size_t i = 0; i < rects.size(); i++)
			{
				VkOffset2D offset = { cmd_scissors[i].offset.x, cmd_scissors[i].offset.y };
				VkExtent2D extent = { cmd_scissors[i].extent.x, cmd_scissors[i].extent.y };
				rects[i] = { offset, extent };
			}

			vkCmdSetScissor(queueData.command_buffer, 0, (uint32_t)rects.size(), rects.data());
		}
		else
		{
			const auto& frameBuffer = VulkanManagerFrameBuffer::Get()->GetFramebuffers(cmd->GetFrameBufferId())[queueData.num_images];
			VkRect2D scissor = CreatorGraphicsPipeline::CreateRect2D(frameBuffer->extent);
			vkCmdSetScissor(queueData.command_buffer, 0, 1, &scissor);
		}
	}

	void Commands::setLineWidth(std::shared_ptr<CmdSetLineWidth> cmd, QueueData& queueData)
	{
		vkCmdSetLineWidth(queueData.command_buffer, cmd->GetWidth());
	}

	void Commands::bindPipeline(std::shared_ptr<CmdBindPipeline> cmd, QueueData& queueData)
	{
		const auto& pipelineId = cmd->GetPipelineId();

		queueData.id_pipeline = pipelineId;

		const auto pipeline = VulkanManagerPipeline::Get()->GetPipeline(pipelineId)->pipeline;

		vkCmdBindPipeline(queueData.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void Commands::bindIndexBuffer(std::shared_ptr<CmdBindIndexBuffer> cmd, QueueData& queueData)
	{
		const auto currentIndexBuffer = VulkanManagerBuffer::Get()->GetIndexBuffer(cmd->GetIndexBufferId());

		auto indexBuffer = currentIndexBuffer->buf_data->buffer;
		auto indexType = currentIndexBuffer->index_type;

		vkCmdBindIndexBuffer(queueData.command_buffer, indexBuffer, 0, indexType);
	}

	void Commands::bindVertexBuffers(std::shared_ptr<CmdBindVertexBuffers> cmd, QueueData& queueData)
	{
		const auto& cmd_vertex_buffers_id = cmd->GetVertexBuffersId();

		const auto numVertexBuffers = cmd_vertex_buffers_id.size();
		VkDeviceSize offsets[] = { 0 };

		auto managerBuffer = VulkanManagerBuffer::Get();
		for (size_t i = 0; i < numVertexBuffers; i++)
		{
			auto renderVertexBuffer = managerBuffer->GetVertexBuffer(cmd_vertex_buffers_id[i]);
			auto vertexBuffer = renderVertexBuffer->buf_data->buffer;
			vkCmdBindVertexBuffers(
				queueData.command_buffer,
				static_cast<uint32_t>(i),
				static_cast<uint32_t>(1),
				&vertexBuffer,
				offsets);
		}
	}

	void Commands::bindDescriptorSets(std::shared_ptr<CmdBindDescriptorSets> cmd, QueueData& queueData)
	{
		const auto& cmd_descriptor_sets_id = cmd->GetDescriptorSetsId();

		const auto numDescriptorSets = cmd_descriptor_sets_id.size();
		std::vector<VkDescriptorSet> descriptorSet(numDescriptorSets);
		for (size_t i = 0; i < numDescriptorSets; i++)
		{
			descriptorSet[i] = VulkanManagerUniformBuffer::Get()->GetUniformDescriptorSet(cmd_descriptor_sets_id[i])[queueData.num_images];
		}

		const auto pipelineLayout = VulkanManagerPipeline::Get()->GetPipelineLayoutData(cmd->GetPipelineId())->pipline_layout;

		vkCmdBindDescriptorSets(
			queueData.command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			static_cast<uint32_t>(0),
			static_cast<uint32_t>(descriptorSet.size()),
			descriptorSet.data(),
			static_cast<uint32_t>(0),
			nullptr);
	}

	void Commands::draw(std::shared_ptr<CmdDraw> cmd, QueueData& queueData)
	{
		vkCmdDraw(queueData.command_buffer, cmd->GetVertexCount(), 1, 0, 0);
	}

	void Commands::drawIndexed(std::shared_ptr<CmdDrawIndexed> cmd, QueueData& queueData)
	{
		const auto currentIndexBuffer = VulkanManagerBuffer::Get()->GetIndexBuffer(cmd->GetIndexBufferId());

		vkCmdDrawIndexed(queueData.command_buffer, currentIndexBuffer->num_index, 1, 0, 0, 0);
	}

	void Commands::drawInderect(std::shared_ptr<CmdDrawInderect> cmd, QueueData& queueData)
	{
		auto id = reinterpret_cast<VkBuffer*>(cmd->buffer_id[queueData.num_images]);
		vkCmdDrawIndirect(
			queueData.command_buffer,
			*id,
			0,
			100,
			sizeof(VkDrawIndexedIndirectCommand)
		);
	}

	void Commands::dispatch(std::shared_ptr<CmdDispatch> cmd, QueueData& queueData)
	{
		vkCmdDispatch(queueData.command_buffer, cmd->GetX(), cmd->GetY(), cmd->GetZ());
	}

	void Commands::traceRays(std::shared_ptr<CmdTraceRays> cmd, QueueData& queueData)
	{
		const DetailRayTracingPass& pass = ManagerRayTracing::Get()->GetPass(cmd->GetWindowId());

		const VkImage outputImage = ManagerSwapchain::Get()->GetImage(pass.swapchain_id, queueData.num_images);

		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		auto transition = [&](
			const VkImageLayout oldLayout,
			const VkImageLayout newLayout,
			const VkAccessFlags srcAccess,
			const VkAccessFlags dstAccess,
			const VkPipelineStageFlags srcStage,
			const VkPipelineStageFlags dstStage)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = outputImage;
			barrier.subresourceRange = subresourceRange;
			barrier.srcAccessMask = srcAccess;
			barrier.dstAccessMask = dstAccess;

			vkCmdPipelineBarrier(
				queueData.command_buffer,
				srcStage,
				dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier);
		};

		// The previous contents of the swapchain image are not read, so the old layout is
		// undefined and the driver is free to discard them.
		transition(
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_GENERAL,
			0,
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

		vkCmdBindPipeline(
			queueData.command_buffer,
			VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
			pass.pipeline->pipeline);

		vkCmdBindDescriptorSets(
			queueData.command_buffer,
			VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
			pass.pipeline_layout->pipline_layout,
			0,
			1,
			&pass.descriptor_sets[queueData.num_images],
			0,
			nullptr);

		ExtensionFunctions::CmdTraceRays(
			queueData.command_buffer,
			&pass.shader_binding_table.raygen_region,
			&pass.shader_binding_table.miss_region,
			&pass.shader_binding_table.hit_region,
			&pass.shader_binding_table.callable_region,
			pass.extent.width,
			pass.extent.height,
			1);

		transition(
			VK_IMAGE_LAYOUT_GENERAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_ACCESS_SHADER_WRITE_BIT,
			0,
			VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
	}

	void Commands::clearColorImage(std::shared_ptr<CmdClearColorImage> cmd, QueueData& queueData)
	{
		static VkImageSubresourceRange subresRange{};

		subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresRange.levelCount = 1;
		subresRange.layerCount = 1;

		static VkClearColorValue clearColor{};
		clearColor.uint32[0] = 0xffffffff;

		const auto& cmd_textures_id = cmd->GetTexturesId();

		vkCmdClearColorImage(
			queueData.command_buffer,
			VulkanManagerTextures::Get()->GetImageData(cmd_textures_id[queueData.num_images])->image,
			VK_IMAGE_LAYOUT_GENERAL,
			&clearColor,
			1,
			&subresRange);
	}

	void Commands::test(std::shared_ptr<CmdTest> cmd, QueueData& queueData)
	{
		cmd->func_(queueData.num_images);
	}
}
