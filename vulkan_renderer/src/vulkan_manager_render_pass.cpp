#include "vulkan_manager_render_pass.h"

#include "creator_render_pass.h"

#include <generator_id.h>
#include <guard_next_id.h>
#include <logger_instance.h>

namespace render
{
	VulkanManagerRenderPass::VulkanManagerRenderPass()
	{

	}

	std::shared_ptr<VulkanManagerRenderPass>& VulkanManagerRenderPass::Get()
	{
		static std::shared_ptr<VulkanManagerRenderPass> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<VulkanManagerRenderPass>(new VulkanManagerRenderPass());
		}
		return manager;
	}

	VulkanManagerRenderPass::~VulkanManagerRenderPass()
	{
		LOG(Loglvl::debug, "[VulkanManagerRenderPass::~VulkanManagerRenderPass]");
	}

	RenderPassId VulkanManagerRenderPass::CreateRenderPass(const render::GraphicsWindowId& windowId,
		const description::RenderPassCreateInfo& renderPassCreateInfo)
	{
		return createRenderPass(
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			renderPassCreateInfo);
	}

	void VulkanManagerRenderPass::DeleteRenderPass(const RenderPassId& renderPassId)
	{
		render_passes_[renderPassId] = nullptr;
		detail_render_pass_[renderPassId] = {};
		next_render_pass_id_ = std::min(size_t(renderPassId), next_render_pass_id_);
		LOG(Loglvl::debug, "[VulkanManagerRenderPass::DeleteRenderPass]", renderPassId.GetId());
	}

	RenderPassId VulkanManagerRenderPass::findRenderPass(
		const LogicalDeviceId& logicalDeviceId,
		const description::RenderPassType type) const
	{
		for (size_t i = 0; i < detail_render_pass_.size(); i++)
		{
			const RenderPassId renderPassId = GeneratorId::GenerateUniqueId<RenderPassId>(i);
			if (render_passes_[renderPassId] && type == detail_render_pass_[renderPassId].type)
			{
				return renderPassId;
			}
		}

		return GeneratorId::GenerateInvalidId<RenderPassId>();
	}

	std::shared_ptr<DataRenderPass> VulkanManagerRenderPass::GetRenderPass(const RenderPassId& renderPassId) const
	{
		return render_passes_[renderPassId];
	}

	const description::RenderPassCreateInfo& VulkanManagerRenderPass::GetDetailRenderPass(const RenderPassId& renderPassId) const
	{
		return detail_render_pass_[renderPassId];
	}

	void VulkanManagerRenderPass::SetClearColorValues(const RenderPassId& renderPassId, const std::vector<glm::vec4>& clearColors)
	{
		detail_render_pass_[renderPassId].clearColors = clearColors;
		detail_render_pass_[renderPassId].clearColorsResolve = clearColors;
	}

	RenderPassId VulkanManagerRenderPass::createRenderPass(
		const LogicalDeviceId& logicalDeviceId,
		const description::RenderPassCreateInfo& renderPassCreateInfo)
	{
		RenderPassId idCopy = findRenderPass(logicalDeviceId, renderPassCreateInfo.type);

		const RenderPassId renderPassId = GeneratorId::GenerateUniqueId<RenderPassId>(next_render_pass_id_);

		auto guardResize = utils::GuardResize::MayBeResize(
			next_render_pass_id_,
			[&](std::shared_ptr<DataRenderPass> renderPass) { return !renderPass; },
			render_passes_,
			detail_render_pass_);

		if (idCopy.IsValid())
		{
			render_passes_[renderPassId] = render_passes_[idCopy];
			detail_render_pass_[renderPassId] = detail_render_pass_[idCopy];
			return renderPassId;
		}

		render_passes_[renderPassId] = CreatorRenderPass::CreateRenderPass(
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			renderPassCreateInfo.colorAttachmentDescriptions,
			renderPassCreateInfo.colorAttachmentDescriptionsResolve,
			renderPassCreateInfo.depthAttachmentDescription);

		detail_render_pass_[renderPassId] = renderPassCreateInfo;

		LOG(Loglvl::debug, "[VulkanManagerRenderPass::CreateRenderPass]", renderPassId.GetId());
		return renderPassId;
	}

}
