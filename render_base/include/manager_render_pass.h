#pragma once

#include "graphics_id.h"
#include "manager_buffer.h"
#include "renderpass_create_info.h"


namespace render
{
	class ManagerRenderPass
	{
	public:
		[[nodiscard]] virtual RenderPassId CreateRenderPass(
			const render::GraphicsWindowId& windowId,
			const description::RenderPassCreateInfo& renderPassCreateInfo) = 0;

		virtual void DeleteRenderPass(const RenderPassId& renderPassId) = 0;

		virtual void SetClearColorValues(
			const RenderPassId& renderPassId,
			const std::vector<glm::vec4>& clearColors) = 0;

		virtual const description::RenderPassCreateInfo& GetDetailRenderPass(const RenderPassId& renderPassId) const = 0;
	};

}
