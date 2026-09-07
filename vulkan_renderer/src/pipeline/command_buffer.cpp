#include "command_buffer.h"

#include "creator_command_buffer.h"
#include "creator_graphics_pipeline.h"
#include "vulkan_manager_drawcall.h"

#include <generator_id.h>
#include <logger_instance.h>
#include <guard_next_id.h>

#include <algorithm>
#include <iostream>

namespace render
{
	CommandBuffer::CommandBuffer(size_t numImages)
	{
		num_images_ = numImages;
		command_buffers_.resize(numImages);
		command_buffers_for_draw_.resize(numImages);
	}

	std::vector<VkCommandBuffer> CommandBuffer::GetCommandBuffer(const CommandBufferId& commandBufferId)
	{
		std::vector<VkCommandBuffer> res;
		for (auto& buffers : command_buffers_)
		{
			res.push_back(buffers[commandBufferId]);
		}
		return res;
	}

	const std::vector<CommandBufferId>& CommandBuffer::GetIdsCommandBuffer() const noexcept
	{
		return ids_command_buffers_;
	}

	const std::vector<VkCommandBuffer>& CommandBuffer::GetCommandBufferForDraw(const size_t numImage)
	{
		//TO DO
		for (auto& uninitializedCommandBuffer : uninitialized_command_buffers_)
		{
			if (uninitializedCommandBuffer.second[numImage])
			{
				const auto& currentId = uninitializedCommandBuffer.first;
				CreatorCommandBuffer::ResetCommandBuffer(command_buffers_[numImage][currentId]);
				CreatorCommandBuffer::RecreateCommandBuffer(
					command_buffers_[numImage][currentId],
					detail_command_buffer_[currentId.GetId()].commandPool,
					detail_command_buffer_[currentId.GetId()].extent);

				initCommandBuffer(currentId, numImage);

				uninitializedCommandBuffer.second[numImage] = false;
			}
		}
		return command_buffers_for_draw_[numImage];
	}

	const size_t CommandBuffer::GetNumCommandBufferForDraw(const size_t numImage)
	{
		return command_buffers_for_draw_[numImage].size();
	}

	size_t CommandBuffer::GetNumFrameBuffers() const noexcept
	{
		return command_buffers_.size();
	}

	CommandBufferId CommandBuffer::CreateCommandBuffer(
		std::shared_ptr<DataCommandPool> commandPool,
		const VkExtent2D& extent)
	{
		auto newId = GeneratorId::GenerateUniqueId<CommandBufferId>(next_id_);

		auto guardId = utils::GuardResize::MayBeResize(
			next_id_,
			[&](const DetailCommandBuffer& detail) { return !detail.commandPool; },
			detail_command_buffer_,
			using_command_buffers_);
		
		for (size_t i = 0; i < num_images_; i++)
		{
			size_t localNextId = next_id_;
			auto localGuardId = utils::GuardResize::MayBeResize(
				localNextId,
				[&](const VkCommandBuffer& commandBuffer) { return commandBuffer == VK_NULL_HANDLE; },
				command_buffers_[i]);
		}

		auto commandBuffers = CreatorCommandBuffer::CreateCommandBuffer(
			commandPool,
			extent,
			num_images_);

		using_command_buffers_[newId] = false;
		for (size_t i = 0; i < num_images_; i++)
		{
			command_buffers_[i][newId] = commandBuffers[i];
		}

		detail_command_buffer_[next_id_] = { commandPool, extent };
		uninitialized_command_buffers_.emplace(std::pair<CommandBufferId, std::vector<bool>>(newId, std::vector<bool>(num_images_, true)));
		ids_command_buffers_.push_back(newId);

		return newId;
	}

	void CommandBuffer::AddDrawcallInCommandBuffer(const DrawcallId& drawcall, const CommandBufferId& commandBufferId)
	{
		uninitialized_command_buffers_[commandBufferId] = std::vector<bool>(num_images_, true);
		drawcalls_[commandBufferId].push_back(drawcall);
	}

	void CommandBuffer::AddToDrawingQueue(const CommandBufferId& commandBufferId, const DrawPriority drawPriority)
	{
		if (using_command_buffers_[commandBufferId])
		{
			return;
		}

		using_command_buffers_[commandBufferId] = true;
		size_t pos = 0;
		for (const auto& priority : draw_priority_)
		{
			if (drawPriority < priority)
			{
				draw_priority_.insert(draw_priority_.begin() + pos, drawPriority);
				break;
			}
			pos++;
		}

		if (draw_priority_.empty() || pos == draw_priority_.size())
		{
			draw_priority_.insert(draw_priority_.begin() + pos, drawPriority);
		}

		for (size_t i = 0; i < command_buffers_for_draw_.size(); i++)
		{
			command_buffers_for_draw_[i].insert(command_buffers_for_draw_[i].begin() + pos, command_buffers_[i][commandBufferId]);
		}
	}

	void CommandBuffer::RecreateCommandBuffer(const CommandBufferId& commandBufferId)
	{
		uninitialized_command_buffers_[commandBufferId] = std::vector<bool>(num_images_, true);
	}

	void CommandBuffer::RecreateCommandBuffers()
	{
		for (const auto& id : ids_command_buffers_)
		{
			RecreateCommandBuffer(id);
		}
	}
	
	bool CommandBuffer::DeleteBindingFromBuffer(DrawcallId& id)
	{
		bool result = false;

		for (auto& [idCommandBuffer, idDrawcall] : drawcalls_)
		{
			auto find = std::find(idDrawcall.begin(), idDrawcall.end(), id);
			if (find != idDrawcall.end())
			{
				result = true;
				RecreateCommandBuffer(idCommandBuffer);
				idDrawcall.erase(find);
			}
		}

		std::erase_if(drawcalls_, [&](const auto& item)
		{
			auto const& [idCommandBuffer, idDrawcall] = item;
			return idDrawcall.empty();
		});

		id = render::GeneratorId::GenerateInvalidId<DrawcallId>();

		return result;
	}

	void CommandBuffer::RemoveFromDrawingQueue(const CommandBufferId& commandBufferId)
	{
		size_t pos = 0;
		const auto& currentCommandBuffer = command_buffers_[0][commandBufferId];
		for (size_t i = 0; i < command_buffers_for_draw_[0].size(); i++)
		{
			if (currentCommandBuffer == command_buffers_for_draw_[0][i])
			{
				for (size_t j = 0; j < command_buffers_for_draw_.size(); j++)
				{
					command_buffers_for_draw_[j].erase(command_buffers_for_draw_[j].begin() + i);
				}
				draw_priority_.erase(draw_priority_.begin() + pos);
				return;
			}
			pos++;
		}
	}

	void CommandBuffer::DeleteCommandBuffer(const CommandBufferId& commandBufferId)
	{
		for (auto& curImageBuf : command_buffers_)
		{
			CreatorCommandBuffer::ResetCommandBuffer(curImageBuf[commandBufferId]);
			curImageBuf[commandBufferId] = VK_NULL_HANDLE;
		}
		uninitialized_command_buffers_.erase(commandBufferId);
		detail_command_buffer_[commandBufferId] = DetailCommandBuffer();
		RemoveFromDrawingQueue(commandBufferId);

		ids_command_buffers_.erase(std::remove(ids_command_buffers_.begin(), ids_command_buffers_.end(), commandBufferId));

		next_id_ = commandBufferId.GetId();
	}

	void CommandBuffer::initCommandBuffer(const CommandBufferId& commandBufferId, const size_t numImages)
	{
		QueueData queueData{};
		queueData.command_buffer = command_buffers_[numImages][commandBufferId];
		queueData.num_images = numImages;

		if (drawcalls_.contains(commandBufferId))
		{
			for (const auto& idDrawcall : drawcalls_[commandBufferId])
			{
				auto drawCall = VulkanManagerDrawcall::Get()->GetDrawcall(idDrawcall);
				for (auto& cmd : drawCall->GetCommands())
				{
					Commands::AddCommand(cmd, queueData);
				}
			}
		}

		CreatorCommandBuffer::EndCommandBuffer(queueData.command_buffer);
	}
}
