#pragma once

#include "struct_data.h"
#include "manager_device.h"
#include "manager_render_pass.h"
#include "manager_window.h"
#include "manager_base.h"

#include <renderpass_create_info.h>
#include <graphics_id.h>
#include <buffer_description.h>
#include <glm/glm.hpp>

#include <vector.h>


namespace render
{
	struct DetailRenderPass
	{
		RenderId logical_device_id;
		description::RenderPassType type;
		std::vector<VkClearValue> clearValues;
		std::vector<description::RenderPassAttachmentDescription> colorAttachmentDescriptions;
		std::vector<description::RenderPassAttachmentDescription> colorAttachmentDescriptionsResolve;
		std::optional<description::RenderPassAttachmentDescription> depthAttachmentDescription;
	};

	class VulkanManagerRenderPass : public ManagerRenderPass, public ManagerBase
	{
	public:
		static std::shared_ptr<VulkanManagerRenderPass>& Get();

		~VulkanManagerRenderPass();

		VulkanManagerRenderPass(const VulkanManagerRenderPass&) = delete;
		VulkanManagerRenderPass(VulkanManagerRenderPass&&) = delete;

		VulkanManagerRenderPass& operator= (const VulkanManagerRenderPass&) = delete;
		VulkanManagerRenderPass& operator= (VulkanManagerRenderPass&&) = delete;

		[[nodiscard]] RenderPassId CreateRenderPass(
			const render::GraphicsWindowId& windowId,
			const description::RenderPassCreateInfo& renderPassCreateInfo) override;

		void DeleteRenderPass(const RenderPassId& renderPassId) override;

		[[nodiscard]] std::shared_ptr<DataRenderPass> GetRenderPass(const RenderPassId& renderPassId) const;

		[[nodiscard]] const description::RenderPassCreateInfo& GetDetailRenderPass(const RenderPassId& renderPassId) const override;

		void SetClearColorValues(const RenderPassId& renderPassId, const std::vector<glm::vec4>& clearColors) override;

	private:
		VulkanManagerRenderPass();

		[[nodiscard]] RenderPassId findRenderPass(
			const LogicalDeviceId& logicalDeviceId,
			const description::RenderPassType type) const;

		[[nodiscard]] RenderPassId createRenderPass(
			const LogicalDeviceId& logicalDeviceId,
			const description::RenderPassCreateInfo& renderPassCreateInfo);

		size_t next_render_pass_id_ = 0;

		Vector<std::shared_ptr<DataRenderPass>, RenderPassId> render_passes_;
		Vector<description::RenderPassCreateInfo, RenderPassId> detail_render_pass_;
	};
}
