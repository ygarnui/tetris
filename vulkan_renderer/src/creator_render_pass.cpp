#include "creator_render_pass.h"

#include "converter_description.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{

std::shared_ptr<DataRenderPass> CreatorRenderPass::CreateRenderPass(
	std::shared_ptr<DataDevice> device,
	const std::vector<description::RenderPassAttachmentDescription>& colorAttachmentDescriptions,
	const std::vector<description::RenderPassAttachmentDescription>& colorAttachmentDescriptionsResolve,
	const std::optional<description::RenderPassAttachmentDescription> depthAttachmentDescription)
{
	std::shared_ptr<DataRenderPass> renderPass(
		new DataRenderPass{ {}, device },
		[](DataRenderPass* p)
		{
			vkDestroyRenderPass(p->device->device, p->render_pass, nullptr);
			delete p;
		}
	);

	std::vector<VkAttachmentReference> colorAttachmentRefs;
	std::vector<VkAttachmentReference> colorAttachmentResolveRefs;
	std::optional<VkAttachmentReference> depthAttachmentRef;

	uint32_t attachmentIndex = 0;
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	for (const auto& colorAttachment : colorAttachmentDescriptions)
	{
		attachmentDescriptions.emplace_back(createColorAttachmentDescription(
			ConverterDescription::FormatToVkFormat(colorAttachment.format),
			ConverterDescription::ConvertImageLayout(colorAttachment.initial_layout),
			ConverterDescription::ConvertImageLayout(colorAttachment.final_layout),
			ConverterDescription::ConvertAttachmentLoadOp(colorAttachment.load_op),
			ConverterDescription::ConvertAttachmentStoreOp(colorAttachment.store_op),
			ConverterDescription::ConvertAttachmentSamples(colorAttachment.samples)));

		colorAttachmentRefs.emplace_back(createColorAttachmentReference(attachmentIndex));
		
		attachmentIndex++;
	}

	for (const auto& colorAttachment : colorAttachmentDescriptionsResolve)
	{
		attachmentDescriptions.emplace_back(createColorAttachmentDescription(
			ConverterDescription::FormatToVkFormat(colorAttachment.format),
			ConverterDescription::ConvertImageLayout(colorAttachment.initial_layout),
			ConverterDescription::ConvertImageLayout(colorAttachment.final_layout),
			ConverterDescription::ConvertAttachmentLoadOp(colorAttachment.load_op),
			ConverterDescription::ConvertAttachmentStoreOp(colorAttachment.store_op),
			ConverterDescription::ConvertAttachmentSamples(colorAttachment.samples)));

		colorAttachmentResolveRefs.emplace_back(createColorAttachmentReference(attachmentIndex));

		attachmentIndex++;
	}

	if (depthAttachmentDescription)
	{
		attachmentDescriptions.emplace_back(createDepthAttachmentDescription(
			ConverterDescription::FormatToVkFormat(depthAttachmentDescription->format),
			ConverterDescription::ConvertImageLayout(depthAttachmentDescription->initial_layout),
			ConverterDescription::ConvertImageLayout(depthAttachmentDescription->final_layout),
			ConverterDescription::ConvertAttachmentLoadOp(depthAttachmentDescription->load_op),
			ConverterDescription::ConvertAttachmentSamples(depthAttachmentDescription->samples)));

		depthAttachmentRef = createDepthAttachmentReference(attachmentIndex);

		attachmentIndex++;
	}

	std::vector<VkSubpassDescription> subpassDescriptions = { createSubpassDescription(colorAttachmentRefs, colorAttachmentResolveRefs, depthAttachmentRef) };
	std::vector<VkSubpassDependency> subpassDependencies = { createSubpassDependency() };
	const VkRenderPassCreateInfo renderPassCreateInfo = createRenderPassCreateInfo(
		attachmentDescriptions,
		subpassDescriptions,
		subpassDependencies);

	const VkResult result = vkCreateRenderPass(device->device, &renderPassCreateInfo, nullptr, &renderPass->render_pass);

	if (result != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorRenderPass::CreateRenderPass] failed to create render pass!");
	}

	return renderPass;
}

VkAttachmentDescription CreatorRenderPass::createColorAttachmentDescription(
	VkFormat format,
	VkImageLayout initialLayout,
	VkImageLayout finalLayout,
	VkAttachmentLoadOp loadOp,
	VkAttachmentStoreOp storeOp,
	VkSampleCountFlagBits samples)
{
	VkAttachmentDescription colorAttachment{};

	colorAttachment.format = format;
	colorAttachment.samples = samples;

	colorAttachment.loadOp = loadOp;
	colorAttachment.storeOp = storeOp;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = initialLayout;
	
	// In case if we wanna present this attachment image to swapchain we need to declare VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	// so in the end of the renderpass this attachment image will be transitioned to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
	// Otherwise if we wanna use this attachment image as a sampled image we need to declare VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
	// There is one more case like use this attachment image just as an attachment during the full render, so it would have a sense to declare
	// it as VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, but there is a VkAttachmentReference to each of the VkAttachmentDescription
	// and this information should be declared there, so in the beginning of the renderpass the image layout is usually declared undefined
	// and during the subpass the correct image layout will be taken from VkAttachmentReference and the attachment image will be transitioned.

	// So for example for a color attachment that will be used as a texture transitions during the renderpass will look like this.

	// VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// Initial					 -> Subpass									 -> Final
	colorAttachment.finalLayout = finalLayout;

	return colorAttachment;
}

VkAttachmentDescription CreatorRenderPass::createDepthAttachmentDescription(
	const VkFormat format,
	const VkImageLayout initialLayout,
	const VkImageLayout finalLayout,
	const VkAttachmentLoadOp loadOp,
	VkSampleCountFlagBits samples)
{
	VkAttachmentDescription depthAttachment{};

	depthAttachment.format = format;
	depthAttachment.samples = samples;

	depthAttachment.loadOp = loadOp;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	depthAttachment.initialLayout = initialLayout;

	// Consider that the depth attachment image is used just as an input attachment.
	depthAttachment.finalLayout = finalLayout;

	return depthAttachment;
}

VkAttachmentReference CreatorRenderPass::createColorAttachmentReference(const uint32_t attachmentIndex)
{
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = attachmentIndex;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	return colorAttachmentRef;
}

VkAttachmentReference CreatorRenderPass::createDepthAttachmentReference(const uint32_t attachmentIndex)
{
	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = attachmentIndex;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	return depthAttachmentRef;
}

VkSubpassDescription CreatorRenderPass::createSubpassDescription(
	const std::vector<VkAttachmentReference>& colorAttachmentRefs,
	const std::vector<VkAttachmentReference>& colorAttachmentResolveRefs,
	const std::optional<VkAttachmentReference>& depthAttachmentRef)
{
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
	subpass.pColorAttachments = colorAttachmentRefs.data();
	if (colorAttachmentResolveRefs.empty())
	{
		subpass.pResolveAttachments = nullptr;
	}
	else
	{
		subpass.pResolveAttachments = colorAttachmentResolveRefs.data();
	}

	subpass.pDepthStencilAttachment = depthAttachmentRef ? &*depthAttachmentRef : VK_NULL_HANDLE;

	return subpass;
}

VkRenderPassCreateInfo CreatorRenderPass::createRenderPassCreateInfo(
	const std::vector<VkAttachmentDescription>& attachmentDescriptions,
	const std::vector<VkSubpassDescription>& subpassDescriptions,
	const std::vector<VkSubpassDependency>& subpassDependencies)
{
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassInfo.pAttachments = attachmentDescriptions.data();

	renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
	renderPassInfo.pSubpasses = subpassDescriptions.data();

	renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassInfo.pDependencies = subpassDependencies.data();

	return renderPassInfo;
}

VkSubpassDependency CreatorRenderPass::createSubpassDependency()
{
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	return dependency;
}

}
