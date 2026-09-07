#pragma once

#include "graphics_id.h"

#include <functional>
#include <glm/glm.hpp>
#include <memory>

namespace render
{
	class ManagerFrameBuffer
	{
	public:
		[[nodiscard]] virtual FrameBufferId CreateRenderPassWindowTarget(
			const std::function<std::pair<uint32_t, uint32_t>(uint32_t width, uint32_t height)>& getSize,
			const RenderPassId& renderPassId,
			const GraphicsWindowId& windowId) = 0;

		[[nodiscard]] virtual FrameBufferId CreateRenderPassTarget(
			const std::function<std::pair<uint32_t, uint32_t>(uint32_t width, uint32_t height)>& getSize,
			const RenderPassId& renderPassId,
			const GraphicsWindowId& windowId) = 0;

		/*!
		\brief Delete the frame buffer and its texture attachments.
		\param[in] frameBufferId will be invalidated.
		*/
		virtual void DeleteFramebuffer(FrameBufferId& frameBufferId) = 0;

		[[nodiscard]] virtual FrameBufferId GetFrameBufferIdBySwapchainId(const SwapchainId& swapchainId) = 0;

		[[nodiscard]] virtual const std::vector<std::vector<TextureId>>& GetFrameBufferAttachments(const FrameBufferId& frameBufferId) const = 0;

		[[nodiscard]] virtual TextureId GetFrameBufferAttachment(
			const FrameBufferId& frameBufferId,
			const size_t imageIndex,
			const size_t index) const = 0;

		/*!
		\brief Clear and create the frame buffer and its texture attachments.
		For example, if the swapchain is resized, its frame buffer must also be recreated.
		*/
		virtual void RecreateFrameBuffer(const FrameBufferId& frameBufferId) = 0;

		virtual bool IsFrameBufferOffScreen(const FrameBufferId& frameBufferId) const = 0;

		virtual void AddForResizeOffScreenFrameBuffersByWindow(const GraphicsWindowId& windowId) = 0;

		virtual void DeleteOffScreenFrameBuffer(FrameBufferId& frameBufferId) = 0;

	};
}
