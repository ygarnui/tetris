#pragma once

#include "struct_data.h"

#include <buffer_description.h>

#include <glm/glm.hpp>

#include <optional>
#include <vector>

namespace render
{
class CreatorRenderPass
{
public:

	[[nodiscard]] static std::shared_ptr<DataRenderPass> CreateRenderPass(
		std::shared_ptr<DataDevice> device,
		const std::vector<description::RenderPassAttachmentDescription>& colorAttachmentDescriptions,
		const std::vector<description::RenderPassAttachmentDescription>& colorAttachmentDescriptionsResolve,
		const std::optional<description::RenderPassAttachmentDescription> depthAttachmentDescription);

private:

	[[nodiscard]] static VkAttachmentDescription createColorAttachmentDescription(
		VkFormat format,
		VkImageLayout initialLayout,
		VkImageLayout finalLayout,
		VkAttachmentLoadOp loadOp,
		VkAttachmentStoreOp storeOp,
		VkSampleCountFlagBits samples);

	[[nodiscard]] static VkAttachmentDescription createDepthAttachmentDescription(
		const VkFormat format,
		const VkImageLayout initialLayout,
		const VkImageLayout finalLayout,
		const VkAttachmentLoadOp loadOp,
		VkSampleCountFlagBits samples);

	[[nodiscard]] static VkAttachmentReference createColorAttachmentReference(const uint32_t attachmentIndex);
	[[nodiscard]] static VkAttachmentReference createDepthAttachmentReference(const uint32_t attachmentIndex);
	
	[[nodiscard]] static VkSubpassDescription createSubpassDescription(
		const std::vector<VkAttachmentReference>& colorAttachmentRefs,
		const std::vector<VkAttachmentReference>& colorAttachmentResolveRefs,
		const std::optional<VkAttachmentReference>& depthAttachmentRef);

	[[nodiscard]] static VkRenderPassCreateInfo createRenderPassCreateInfo(
		const std::vector<VkAttachmentDescription>& attachmentDescription,
		const std::vector<VkSubpassDescription>& subpassDescription,
		const std::vector<VkSubpassDependency>& subpassDependency);

	[[nodiscard]] static VkSubpassDependency createSubpassDependency();
};
}
