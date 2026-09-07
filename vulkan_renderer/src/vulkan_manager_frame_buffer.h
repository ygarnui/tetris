#pragma once

#include "struct_data.h"
#include "vulkan_manager_render_pass.h"
#include "manager_swapchain.h"
#include "manager_device.h"
#include "textures/vulkan_manager_textures.h"
#include "manager_window.h"
#include "manager_base.h"

#include <manager_frame_buffer.h>

#include <vector>
#include <memory>
#include <set>

namespace render
{
	struct DetailFrameBuffer
	{
		std::function<std::pair<uint32_t, uint32_t>(uint32_t width, uint32_t height)> getSize;
		SwapchainId swapchain_id;
		RenderPassId render_pass_id;
		LogicalDeviceId logical_device_id;
		PhysicalDeviceId physical_device_id;
		VkExtent2D extent = { 0, 0 };
		uint32_t num_images = 0;
	};

	class VulkanManagerFrameBuffer : public ManagerFrameBuffer, public ManagerBase
	{
	public:
		static std::shared_ptr<VulkanManagerFrameBuffer>& Get();

		~VulkanManagerFrameBuffer();

		VulkanManagerFrameBuffer(const VulkanManagerFrameBuffer&) = delete;
		VulkanManagerFrameBuffer(VulkanManagerFrameBuffer&&) = delete;

		VulkanManagerFrameBuffer& operator= (const VulkanManagerFrameBuffer&) = delete;
		VulkanManagerFrameBuffer& operator= (VulkanManagerFrameBuffer&&) = delete;

		/*!
		\brief Add a framebuffer to render to it and use its attachments as textures.
		if one of the attachments has present_to_swapchain enabled,
		the attachment will be created from swapchain images and can be presented to swapchain.
		*/
		[[nodiscard]] FrameBufferId CreateRenderPassWindowTarget(
			const std::function<std::pair<uint32_t, uint32_t>(uint32_t width, uint32_t height)>& getSize,
			const RenderPassId& renderPassId,
			const GraphicsWindowId& windowId) override;

		/*!
		\brief Add a framebuffer to render to it and use its attachments as textures.
		If one of the attachments has present_to_swapchain enabled, an exception will be thrown.
		*/
		[[nodiscard]] FrameBufferId CreateRenderPassTarget(
			const std::function<std::pair<uint32_t, uint32_t>(uint32_t width, uint32_t height)>& getSize,
			const RenderPassId& renderPassId,
			const GraphicsWindowId& windowId) override;

		/*!
		\brief Delete the frame buffer and its texture attachments.
		\param[in] frameBufferId will be invalidated.
		*/
		void DeleteFramebuffer(FrameBufferId& frameBufferId) override;

		[[nodiscard]] FrameBufferId GetFrameBufferIdBySwapchainId(const SwapchainId& swapchainId) override;

		/*!
		\brief Look up the frame buffer of a swapchain without treating its absence as an error.

		A ray traced window has no frame buffer at all, because the ray generation shader
		writes into the swapchain image directly. Callers that only want to refresh a frame
		buffer if there is one use this instead of GetFrameBufferIdBySwapchainId, which throws.
		\param[in] swapchainId the swapchain to look up
		\return the frame buffer id, or an invalid id when the swapchain has none
		*/
		[[nodiscard]] FrameBufferId FindFrameBufferIdBySwapchainId(const SwapchainId& swapchainId);

		[[nodiscard]] const std::vector<std::shared_ptr<DataFrameBuffer>>& GetFramebuffers(const FrameBufferId& frameBufferId) const;

		[[nodiscard]] const std::vector<std::vector<TextureId>>& GetFrameBufferAttachments(const FrameBufferId& frameBufferId) const override;

		[[nodiscard]] TextureId GetFrameBufferAttachment(
			const FrameBufferId& frameBufferId,
			const size_t imageIndex,
			const size_t index) const override;

		/*!
		\brief Clear and create the frame buffer and its texture attachments.
		For example, if the swapchain is resized, its frame buffer must also be recreated.
		*/
		void RecreateFrameBuffer(const FrameBufferId& frameBufferId) override;

		void AddForResizeOffScreenFrameBuffersByWindow(const GraphicsWindowId& windowId) override;

		void ApplyResize();

		bool IsFrameBufferOffScreen(const FrameBufferId& frameBufferId) const override;

		void DeleteOffScreenFrameBuffer(FrameBufferId& frameBufferId) override;

	private:
		struct FrameBuffer
		{
			std::vector<std::shared_ptr<DataFrameBuffer>> frame_buffers;
			std::vector<std::vector<TextureId>> attachments;
			std::vector<std::vector<TextureId>> msaa_attachments;
			std::vector<std::vector<TextureId>> depth_attachments;

			DetailFrameBuffer frame_buffer_detail;
		};

		VulkanManagerFrameBuffer();

		[[nodiscard]] FrameBufferId addFrameBuffer(
			const std::function<std::pair<uint32_t, uint32_t>(uint32_t width, uint32_t height)>& getSize,
			const SwapchainId& swapchainId,
			const RenderPassId& renderPassId,
			const LogicalDeviceId& logicalDeviceId,
			const PhysicalDeviceId& physicalDeviceId,
			const VkExtent2D& extent,
			const uint32_t numImages);

		/*!
		\brief Delete frame buffers and its texture attachments
		and keep the FrameBufferId valid for proper recreation.
		*/
		void clear(const FrameBufferId& frameBufferId);

		/*!
		\brief Create color and depth texture attachments
		and then create a framebuffer with these texture attachments image views.
		*/
		void createFrameBuffer(const FrameBufferId& frameBufferId);

		void initImageViewAttachments(
			const std::vector<description::RenderPassAttachmentDescription>& attachments, 
			size_t imageIndex,
			const DetailFrameBuffer& frameBufferDetails,
			FrameBuffer& frameBuffer,
			std::vector<VkImageView>& imageViews);

		TextureId createColorAttachment(
			const LogicalDeviceId& logicalDeviceId,
			const PhysicalDeviceId& physicalDeviceId,
			const VkExtent2D& extent,
			const description::Format format,
			const image::SamplerCreateInfo::AddressMode addressMode,
			VkSampleCountFlagBits sampler);

		TextureId createDepthAttachment(
			const LogicalDeviceId& logicalDeviceId,
			const PhysicalDeviceId& physicalDeviceId,
			const VkExtent2D& extent,
			VkSampleCountFlagBits sampleCountFlagBits);

		size_t next_id_ = 0;

		Vector<FrameBuffer, FrameBufferId> frame_buffers_;

		std::set<SwapchainId> recreate_ids_;
	};
}
