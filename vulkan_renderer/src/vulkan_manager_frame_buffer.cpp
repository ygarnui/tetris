#include "vulkan_manager_frame_buffer.h"

#include "creator_frame_buffer.h"
#include "manager_window.h"
#include "selector_swapchain_settings.h"

#include <interface_image_object.h>
#include <guard_next_id.h>
#include <logger_instance.h>

#include "vulkan_manager_render_pass.h"

namespace render
{
	VulkanManagerFrameBuffer::VulkanManagerFrameBuffer()
	{

	}

	std::shared_ptr<VulkanManagerFrameBuffer>& VulkanManagerFrameBuffer::Get()
	{
		static std::shared_ptr<VulkanManagerFrameBuffer> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<VulkanManagerFrameBuffer>(new VulkanManagerFrameBuffer);
		}
		return manager;
	}

	VulkanManagerFrameBuffer::~VulkanManagerFrameBuffer()
	{
		LOG(Loglvl::debug, "[VulkanManagerFrameBuffer::~VulkanManagerFrameBuffer]");
	}

	FrameBufferId VulkanManagerFrameBuffer::CreateRenderPassWindowTarget(
		const std::function<std::pair<uint32_t, uint32_t>(uint32_t width, uint32_t height)>& getSize,
		const RenderPassId& renderPassId,
		const GraphicsWindowId& windowId)
	{
		if (!getSize)
		{
			LOGEXC(std::invalid_argument, "[VulkanManagerFrameBuffer::CreateRenderPassWindowTarget] getSize not set");
			return GeneratorId::GenerateInvalidId<FrameBufferId>();
		}

		// Extent and num images set here as invalid, because it will be set up in RecreateFrameBuffer(),
		// so when resize a window the framebuffer extent and num images will be updated.
		return addFrameBuffer(
			getSize,
			ManagerWindow::Get()->GetSwapchainId(windowId),
			renderPassId,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerWindow::Get()->GetPhysicalDeviceId(windowId),
			{},
			0);
	}

	FrameBufferId VulkanManagerFrameBuffer::CreateRenderPassTarget(
		const std::function<std::pair<uint32_t, uint32_t>(uint32_t width, uint32_t height)>& getSize,
		const RenderPassId& renderPassId,
		const GraphicsWindowId& windowId)
	{
		if (!getSize)
		{
			LOGEXC(std::invalid_argument, "[VulkanManagerFrameBuffer::CreateRenderPassTarget] getSize not set");
			return GeneratorId::GenerateInvalidId<FrameBufferId>();
		}

		auto size = ManagerSwapchain::Get()->GetSize(ManagerWindow::Get()->GetSwapchainId(windowId));
		auto width = size.x;
		auto height = size.y;
		auto [new_width, new_height] = getSize(width, height);

		return addFrameBuffer(
			getSize,
			GeneratorId::GenerateInvalidId<SwapchainId>(),
			renderPassId,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerWindow::Get()->GetPhysicalDeviceId(windowId),
			VkExtent2D{ new_width, new_height },
			ManagerSwapchain::Get()->GetNumImages(ManagerWindow::Get()->GetSwapchainId(windowId)));
	}

	void VulkanManagerFrameBuffer::DeleteFramebuffer(FrameBufferId& frameBufferId)
	{
		clear(frameBufferId);

		frame_buffers_[frameBufferId].frame_buffer_detail = {};

		next_id_ = std::min(next_id_, frameBufferId.GetId());

		frameBufferId = GeneratorId::GenerateInvalidId<FrameBufferId>();
	}

	FrameBufferId VulkanManagerFrameBuffer::GetFrameBufferIdBySwapchainId(const SwapchainId& swapchainId)
	{
		for (size_t frameBufferIndex = 0; frameBufferIndex < frame_buffers_.size(); frameBufferIndex++)
		{
			FrameBufferId frameBufferId = GeneratorId::GenerateUniqueId<FrameBufferId>(frameBufferIndex);
			if (frame_buffers_[frameBufferId].frame_buffer_detail.swapchain_id == swapchainId)
			{
				return frameBufferId;
			}
		}

		LOGEXC(std::runtime_error, "[VulkanManagerFrameBuffer::GetFrameBufferIdBySwapchainId] Cant find FrameBufferId by swapchainId!");
		return FrameBufferId();
	}

	const std::vector<std::shared_ptr<DataFrameBuffer>>& VulkanManagerFrameBuffer::GetFramebuffers(const FrameBufferId& frameBufferId) const
	{
		return frame_buffers_[frameBufferId].frame_buffers;
	}

	const std::vector<std::vector<TextureId>>& VulkanManagerFrameBuffer::GetFrameBufferAttachments(const FrameBufferId& frameBufferId) const
	{
		return frame_buffers_[frameBufferId].attachments;
	}

	TextureId VulkanManagerFrameBuffer::GetFrameBufferAttachment(
		const FrameBufferId& frameBufferId,
		const size_t imageIndex,
		const size_t index) const
	{
		return frame_buffers_[frameBufferId].attachments[imageIndex][index];
	}

	void VulkanManagerFrameBuffer::RecreateFrameBuffer(const FrameBufferId& frameBufferId)
	{
		clear(frameBufferId);

		FrameBuffer& frameBuffer = frame_buffers_[frameBufferId];
		DetailFrameBuffer& frameBufferDetails = frameBuffer.frame_buffer_detail;

		if (frameBufferDetails.swapchain_id.IsValid())
		{
			frameBufferDetails.extent = ManagerSwapchain::Get()->GetExtent(frameBufferDetails.swapchain_id);
			frameBufferDetails.num_images = ManagerSwapchain::Get()->GetNumImages(frameBufferDetails.swapchain_id);
		}
	
		if (frameBufferDetails.num_images == 0)
		{
			LOGEXC(std::runtime_error, "[VulkanManagerFrameBuffer::RecreateFrameBuffer] Failed to add frame buffer, num images has to be > 0!");
		}

		if (frameBufferDetails.extent.width == 0 || frameBufferDetails.extent.height == 0)
		{
			LOGEXC(std::runtime_error, "[VulkanManagerFrameBuffer::RecreateFrameBuffer] Failed to add frame buffer, extent width and height have to be > 0!");
		}

		createFrameBuffer(frameBufferId);
	}

	void VulkanManagerFrameBuffer::AddForResizeOffScreenFrameBuffersByWindow(const GraphicsWindowId& windowId)
	{
		recreate_ids_.emplace(ManagerWindow::Get()->GetSwapchainId(windowId));
	}

	void VulkanManagerFrameBuffer::ApplyResize()
	{
		for (const auto& swapchainId : recreate_ids_)
		{
			for (size_t frameBufferIndex = 0; frameBufferIndex < frame_buffers_.size(); frameBufferIndex++)
			{
				FrameBufferId frameBufferId = GeneratorId::GenerateUniqueId<FrameBufferId>(frameBufferIndex);
				auto& frameBufferDetail = frame_buffers_[frameBufferId].frame_buffer_detail;
				if (frameBufferDetail.swapchain_id == swapchainId && !frameBufferDetail.swapchain_id.IsValid())
				{
					const auto newSize = ManagerSwapchain::Get()->GetExtent(swapchainId);
					VkExtent2D& oldSize = frameBufferDetail.extent;
					if (newSize.width != oldSize.width || newSize.height != oldSize.height)
					{
						oldSize = newSize;
						createFrameBuffer(frameBufferId);
					}
				}
			}
		}

		recreate_ids_.clear();
	}

	bool VulkanManagerFrameBuffer::IsFrameBufferOffScreen(const FrameBufferId& frameBufferId) const
	{
		const FrameBuffer& frameBuffer = frame_buffers_[frameBufferId];
		return !frameBuffer.frame_buffer_detail.swapchain_id.IsValid();
	}

	void VulkanManagerFrameBuffer::DeleteOffScreenFrameBuffer(FrameBufferId& frameBufferId)
	{
		if (!IsFrameBufferOffScreen(frameBufferId))
		{
			LOGEXC(std::invalid_argument, "[VulkanManagerFrameBuffer::DeleteOffScreenFrameBuffer] Cant delete not offscreen_rendering framebuffer");
		}

		DeleteFramebuffer(frameBufferId);
	}

	FrameBufferId VulkanManagerFrameBuffer::addFrameBuffer(
		const std::function<std::pair<uint32_t, uint32_t>(uint32_t width, uint32_t height)>& getSize,
		const SwapchainId& swapchainId,
		const RenderPassId& renderPassId,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId,
		const VkExtent2D& extent,
		const uint32_t numImages)
	{
		const FrameBufferId frameBufferId = GeneratorId::GenerateUniqueId<FrameBufferId>(next_id_);

		const auto guardId = utils::GuardResize::MayBeResize(
			next_id_,
			[&](const FrameBuffer& buf) { return !buf.frame_buffer_detail.render_pass_id.IsValid(); },
			frame_buffers_);

		DetailFrameBuffer detailFrameBuffer;
		detailFrameBuffer.getSize = getSize;
		detailFrameBuffer.swapchain_id = swapchainId;
		detailFrameBuffer.render_pass_id = renderPassId;
		detailFrameBuffer.logical_device_id = logicalDeviceId;
		detailFrameBuffer.physical_device_id = physicalDeviceId;
		detailFrameBuffer.extent = extent;
		detailFrameBuffer.num_images = numImages;

		frame_buffers_[frameBufferId].frame_buffer_detail = detailFrameBuffer;

		RecreateFrameBuffer(frameBufferId);

		return frameBufferId;
	}

	void VulkanManagerFrameBuffer::clear(const FrameBufferId& frameBufferId)
	{
		FrameBuffer& frameBuffer = frame_buffers_[frameBufferId];
		for (const auto& dataFrameBuffer : frameBuffer.frame_buffers)
		{
			if (dataFrameBuffer.use_count() > 1)
			{
				LOGEXC(std::runtime_error, "[VulkanManagerFrameBuffer::clear] Framebuffer use_count > 1!");
			}
		}
		frameBuffer.frame_buffers.clear();

		for (const auto& attachmentsByImageIndex : frameBuffer.attachments)
		{
			for (auto attachment : attachmentsByImageIndex)
			{
				VulkanManagerTextures::Get()->DeleteTexture(attachment);
			}
		}
		frameBuffer.attachments.clear();
	}

	void VulkanManagerFrameBuffer::createFrameBuffer(const FrameBufferId& frameBufferId)
	{
		FrameBuffer& frameBuffer = frame_buffers_[frameBufferId];
		const DetailFrameBuffer& frameBufferDetails = frameBuffer.frame_buffer_detail;
		const auto& renderPassDetails = VulkanManagerRenderPass::Get()->GetDetailRenderPass(frameBufferDetails.render_pass_id);
		const uint32_t numImages = frameBufferDetails.num_images;
		const VkExtent2D extent = frameBufferDetails.extent;

		frameBuffer.frame_buffers = std::vector<std::shared_ptr<DataFrameBuffer>>(numImages);
		for (const auto& idTextures : frameBuffer.attachments)
		{
			for (const auto id : idTextures)
			{
				VulkanManagerTextures::Get()->DeleteTexture(id);
			}
		}

		for (const auto& idTextures : frameBuffer.msaa_attachments)
		{
			for (const auto id : idTextures)
			{
				VulkanManagerTextures::Get()->DeleteTexture(id);
			}
		}

		for (const auto& idTextures : frameBuffer.depth_attachments)
		{
			for (const auto id : idTextures)
			{
				VulkanManagerTextures::Get()->DeleteTexture(id);
			}
		}
		frameBuffer.attachments = std::vector<std::vector<TextureId>>(numImages);
		frameBuffer.msaa_attachments = std::vector<std::vector<TextureId>>(numImages);
		frameBuffer.depth_attachments = std::vector<std::vector<TextureId>>(numImages);
		for (uint32_t imageIndex = 0; imageIndex < numImages; imageIndex++)
		{
			std::vector<VkImageView> imageViews;
			initImageViewAttachments(
				renderPassDetails.colorAttachmentDescriptions,
				imageIndex,
				frameBufferDetails,
				frameBuffer,
				imageViews);
			initImageViewAttachments(
				renderPassDetails.colorAttachmentDescriptionsResolve,
				imageIndex,
				frameBufferDetails,
				frameBuffer,
				imageViews);

			if (renderPassDetails.depthAttachmentDescription)
			{
				auto depthAttachment = renderPassDetails.depthAttachmentDescription.value();
				const TextureId textureId = createDepthAttachment(
					frameBufferDetails.logical_device_id,
					frameBufferDetails.physical_device_id,
					extent,
					VkSampleCountFlagBits(depthAttachment.samples));

				imageViews.emplace_back(VulkanManagerTextures::Get()->GetImageViewData(textureId)->image_view);
				frameBuffer.depth_attachments[imageIndex].emplace_back(textureId);
			}

			frameBuffer.frame_buffers[imageIndex] = CreatorFrameBuffer::CreateFrameBuffer(
				imageViews,
				VulkanManagerRenderPass::Get()->GetRenderPass(frameBufferDetails.render_pass_id),
				ManagerDevice::Get()->GetLogicalDevice(frameBufferDetails.logical_device_id),
				extent
			);
		}
	}

	void VulkanManagerFrameBuffer::initImageViewAttachments(
		const std::vector<description::RenderPassAttachmentDescription>& attachments,
		size_t imageIndex,
		const DetailFrameBuffer& frameBufferDetails,
		FrameBuffer& frameBuffer,
		std::vector<VkImageView>& imageViews)
	{
		const auto samplCount1Bit = description::SampleCountFlagBits::SAMPLE_COUNT_1_BIT;
		for (const auto& colorAttachment : attachments)
		{
			if (colorAttachment.present_to_swapchain && frameBufferDetails.swapchain_id.IsValid() && ManagerSwapchain::Get()->GetSwapchain(frameBufferDetails.swapchain_id))
			{
				if (colorAttachment.samples == samplCount1Bit)
				{
					const auto& swapchainImageViews = ManagerSwapchain::Get()->GetImageViewData(frameBufferDetails.swapchain_id);
					imageViews.emplace_back(swapchainImageViews[imageIndex]->image_view);
				}
				else
				{
					LOGEXC(std::invalid_argument, "[ManagerFrameBuffer::initImageViewAttachments] the format of the permissive attachment must always be SAMPLE_COUNT_1_BIT");
				}
			}
			else
			{
				const TextureId textureId = createColorAttachment(
					frameBufferDetails.logical_device_id,
					frameBufferDetails.physical_device_id,
					frameBufferDetails.extent,
					colorAttachment.format,
					colorAttachment.address_mode,
					VkSampleCountFlagBits(colorAttachment.samples));

				if (colorAttachment.samples == samplCount1Bit)
				{
					frameBuffer.attachments[imageIndex].emplace_back(textureId);
				}
				else
				{
					frameBuffer.msaa_attachments[imageIndex].emplace_back(textureId);
				}
				imageViews.emplace_back(VulkanManagerTextures::Get()->GetImageViewData(textureId)->image_view);
			}
		}
	}

	TextureId VulkanManagerFrameBuffer::createColorAttachment(
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId,
		const VkExtent2D& extent,
		const description::Format format,
		const image::SamplerCreateInfo::AddressMode addressMode,
		VkSampleCountFlagBits sampler)
	{
		std::shared_ptr<image::InterfaceImageObject> imageSourceData = std::make_shared<image::InterfaceImageObject>();
		imageSourceData->is_mipmaps_enabled = false;
		imageSourceData->tex_channels = 4;
		imageSourceData->tex_width = extent.width;
		imageSourceData->tex_height = extent.height;
		imageSourceData->format = format;
		imageSourceData->usage = description::TextureUsage::COLOR_ATTACHMENT;
		imageSourceData->sampler.addressMode = addressMode;
		return VulkanManagerTextures::Get()->AddTexture(
			imageSourceData,
			sampler,
			logicalDeviceId,
			physicalDeviceId);
	}

	TextureId VulkanManagerFrameBuffer::createDepthAttachment(
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId,
		const VkExtent2D& extent,
		VkSampleCountFlagBits sampleCountFlagBits)
	{
		std::shared_ptr<image::InterfaceImageObject> imageSourceData = std::make_shared<image::InterfaceImageObject>();
		imageSourceData->format = description::Format::D32_SFLOAT;
		imageSourceData->is_mipmaps_enabled = false;
		imageSourceData->tex_channels = 4;
		imageSourceData->tex_width = extent.width;
		imageSourceData->tex_height = extent.height;
		imageSourceData->usage = description::TextureUsage::DEPTH_ATTACHMENT;
		return VulkanManagerTextures::Get()->AddTexture(
			imageSourceData,
			sampleCountFlagBits,
			logicalDeviceId,
			physicalDeviceId);
	}
}
