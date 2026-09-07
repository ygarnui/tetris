#pragma once

#include "command_buffer.h"
#include "../manager_base.h"

#include <manager_command_buffer.h>
#include <vulkan/vulkan.h>

namespace render
{
	class VulkanManagerCommandBuffer : public ManagerCommandBuffer, public ManagerBase
	{
	public:
		static std::shared_ptr<VulkanManagerCommandBuffer>& Get();

		~VulkanManagerCommandBuffer();

		VulkanManagerCommandBuffer(const VulkanManagerCommandBuffer&) = delete;
		VulkanManagerCommandBuffer(VulkanManagerCommandBuffer&&) = delete;

		VulkanManagerCommandBuffer& operator= (const VulkanManagerCommandBuffer&) = delete;
		VulkanManagerCommandBuffer& operator= (VulkanManagerCommandBuffer&&) = delete;

		[[nodiscard]] size_t GetNumberCommandBuffers(const GraphicsWindowId& windowId);

		std::vector<VkCommandBuffer> GetCommandBuffer(
			const GraphicsWindowId& windowId,
			const CommandBufferId& commandBufferId
		);

		[[nodiscard]] std::vector<VkCommandBuffer> GetCommandBuffersByNumImage(
			const GraphicsWindowId& windowId,
			const size_t numImage);

		[[nodiscard]] bool AddWindow(
			const GraphicsWindowId& windowId,
			const size_t numSwapchainImages);

		[[nodiscard]] CommandBufferId CreateCommandBuffer(
			const GraphicsWindowId& windowId,
			const SwapchainId& swapchainId,
			const LogicalDeviceId& logicalDeviceId);

		void AddDrawcallInCommandBuffer(
			const GraphicsWindowId& windowId, 
			const DrawcallId& drawcallId,
			const CommandBufferId& commandBufferId) override;

		void RecreateCommandBuffers(const GraphicsWindowId& windowId) override;

		void AddToDrawingQueue(
			const GraphicsWindowId& windowId,
			const CommandBufferId& commandBufferId,
			const DrawPriority drawPriority) override;

		void RecreateCommandBuffer(
			const GraphicsWindowId& windowId, 
			const CommandBufferId& commandBufferId) override;

		void DeleteFromDrawingQueue(
			const GraphicsWindowId& windowId,
			const CommandBufferId& commandBufferId) override;

		bool DeleteDrawCallFromCommandBuffer(
			const GraphicsWindowId& windowId,
			DrawcallId& drawcallId) override;

		bool DeleteCommandBuffer(
			const GraphicsWindowId& windowId,
			const CommandBufferId& commandBufferId) override;

		void DeleteWindowCommandBuffer(const GraphicsWindowId& windowId) override;

	private:
		VulkanManagerCommandBuffer() = default;
		
		std::map<GraphicsWindowId, std::shared_ptr<CommandBuffer>> command_buffers_;
	};
}
