#pragma once

#include "../struct_data.h"

#include <cmd_calls.h>

#include <logger_instance.h>
#include <memory>

namespace render
{
	struct QueueData
	{
		VkCommandBuffer command_buffer;
		size_t num_images;
		RenderId id_pipeline = RenderId();
		RenderPassId id_render_pass = RenderPassId();
		FrameBufferId id_frame_buffer = FrameBufferId();
		Viewport viewport{};
	};

	class Commands
	{
	public:
		static void AddCommand(std::shared_ptr<CmdBaseAbstact> cmd, QueueData& queueData);
		Commands() = delete;

	private:
		template<typename T>
		static std::shared_ptr<T> commandsConverter(std::shared_ptr<CmdBaseAbstact> cmd)
		{
			const auto command = std::dynamic_pointer_cast<T>(cmd);
			if (!command)
			{
				LOGEXC(std::runtime_error, "[Commands::commandsConverter] cant cast std::shared_ptr<CmdBaseAbstact> to std::dynamic_pointer_cast<T>");
			}
			return command;
		}

		static void beginRenderPass(std::shared_ptr<CmdBeginRenderPass> cmd, QueueData& queueData);
		static void endRenderPass(std::shared_ptr<CmdEndRenderPass> cmd, QueueData& queueData);
		
		static void setViewport(std::shared_ptr<CmdSetViewport> cmd, QueueData& queueData);
		static void setScissor(std::shared_ptr<CmdSetScissor> cmd, QueueData& queueData);
		static void setLineWidth(std::shared_ptr<CmdSetLineWidth> cmd, QueueData& queueData);

		static void bindPipeline(std::shared_ptr<CmdBindPipeline> cmd, QueueData& queueData);
		static void bindIndexBuffer(std::shared_ptr<CmdBindIndexBuffer> cmd, QueueData& queueData);
		static void bindVertexBuffers(std::shared_ptr<CmdBindVertexBuffers> cmd, QueueData& queueData);
		static void bindDescriptorSets(std::shared_ptr<CmdBindDescriptorSets> cmd, QueueData& queueData);
		 
		static void draw(std::shared_ptr<CmdDraw> cmd, QueueData& queueData);
		static void drawIndexed(std::shared_ptr<CmdDrawIndexed> cmd, QueueData& queueData);
		static void drawInderect(std::shared_ptr<CmdDrawInderect> cmd, QueueData& queueData);
		
		static void dispatch(std::shared_ptr<CmdDispatch> cmd, QueueData& queueData);

		static void traceRays(std::shared_ptr<CmdTraceRays> cmd, QueueData& queueData);

		static void clearColorImage(std::shared_ptr<CmdClearColorImage> cmd, QueueData& queueData);
		static void test(std::shared_ptr<CmdTest> cmd, QueueData& queueData);
	};
}
