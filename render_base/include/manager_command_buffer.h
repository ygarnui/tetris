#pragma once

#include "graphics_id.h"
#include "draw_priority.h"

namespace render
{
	class ManagerCommandBuffer
	{
	public:
		virtual void AddDrawcallInCommandBuffer(
			const GraphicsWindowId& windowId, 
			const DrawcallId& drawcallId, 
			const CommandBufferId& commandBufferId) = 0;

		virtual void RecreateCommandBuffers(const GraphicsWindowId& windowId) = 0;

		virtual void AddToDrawingQueue(
			const GraphicsWindowId& windowId,
			const CommandBufferId& commandBufferId,
			const DrawPriority drawPriority) = 0;

		virtual void RecreateCommandBuffer(
			const GraphicsWindowId& windowId, 
			const CommandBufferId& commandBufferId) = 0;

		virtual void DeleteFromDrawingQueue(
			const GraphicsWindowId& windowId,
			const CommandBufferId& commandBufferId) = 0;

		virtual bool DeleteDrawCallFromCommandBuffer(
			const GraphicsWindowId& windowId,
			DrawcallId& drawcallId) = 0;

		virtual bool DeleteCommandBuffer(
			const GraphicsWindowId& windowId,
			const CommandBufferId& commandBufferId) = 0;

		virtual void DeleteWindowCommandBuffer(const GraphicsWindowId& windowId) = 0;
	};
}
