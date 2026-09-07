#pragma once

#include "commands.h"
#include "../struct_data.h"
#include "../vulkan_manager_render_pass.h"
#include "../pipeline/vulkan_manager_pipeline.h"
#include "../buffers/vulkan_manager_buffer.h"
#include "../buffers/vulkan_manager_uniform_buffer.h"

#include <cmd_calls.h>
#include <draw_priority.h>

#include <vector>
#include <map>

namespace render
{
	struct DetailCommandBuffer
	{
		std::shared_ptr<DataCommandPool> commandPool;
		VkExtent2D extent;
	};

	class CommandBuffer
	{
	public:
		CommandBuffer(size_t numImages);

		std::vector<VkCommandBuffer> GetCommandBuffer(const CommandBufferId& commandBufferId);

		[[nodiscard]] const std::vector<CommandBufferId>& GetIdsCommandBuffer() const noexcept;
		[[nodiscard]] const std::vector<VkCommandBuffer>& GetCommandBufferForDraw(const size_t numImage);
		[[nodiscard]] const size_t GetNumCommandBufferForDraw(const size_t numImage);
		[[nodiscard]] size_t GetNumFrameBuffers() const noexcept;

		[[nodiscard]] CommandBufferId CreateCommandBuffer(std::shared_ptr<DataCommandPool> commandPool,	const VkExtent2D& extent);

		void AddDrawcallInCommandBuffer(const DrawcallId& drawcall, const CommandBufferId& commandBufferId);
		void AddToDrawingQueue(const CommandBufferId& commandBufferId, const DrawPriority drawPriority);

		void RecreateCommandBuffer(const CommandBufferId& commandBufferId);
		void RecreateCommandBuffers();

		bool DeleteBindingFromBuffer(DrawcallId& id);
		void RemoveFromDrawingQueue(const CommandBufferId& commandBufferId);
		void DeleteCommandBuffer(const CommandBufferId& commandBufferId);

	private:
		void initCommandBuffer(const CommandBufferId& commandBufferId, const size_t numImages);

		size_t next_id_ = 0;
		
		size_t num_images_;
		std::vector<Vector<VkCommandBuffer, CommandBufferId>> command_buffers_;
		Vector<bool, CommandBufferId> using_command_buffers_;

		std::vector<DetailCommandBuffer> detail_command_buffer_;
		std::map<CommandBufferId, std::vector<bool>> uninitialized_command_buffers_;

		std::vector<DrawPriority> draw_priority_;
		std::vector<std::vector<VkCommandBuffer>> command_buffers_for_draw_;

		std::vector<CommandBufferId> ids_command_buffers_;

		std::map<CommandBufferId, std::vector<DrawcallId>> drawcalls_;
	};
}
