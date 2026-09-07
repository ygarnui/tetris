#include "manager_window.h"

#include "manager_surface.h"
#include "textures/vulkan_manager_textures.h"
#include "vulkan_manager_frame_buffer.h"
#include "manager_swapchain.h"
#include "manager_device.h"

#include <logger_instance.h>
#include <vector_utils.h>
#include <guard_next_id.h>
#include <generator_id.h>

#include <stdexcept>

namespace render
{
	std::shared_ptr<ManagerWindow>& ManagerWindow::Get()
	{
		static std::shared_ptr<ManagerWindow> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<ManagerWindow>(new ManagerWindow());
		}
		return manager;
	}

	ManagerWindow::~ManagerWindow()
	{
		LOG(Loglvl::debug, "[ManagerWindow::~ManagerWindow]");
	}

	GraphicsWindowId ManagerWindow::AddWindow(
		const SwapchainId& swapchainId,
		const SurfaceId& surfaceId,
		const PhysicalDeviceId& physicalDeviceId,
		const LogicalDeviceId& logicalDeviceId)
	{
		auto newId = GeneratorId::GenerateUniqueId<GraphicsWindowId>();
		
		DetailWindow detailWindow;
		detailWindow.id_window = newId;
		detailWindow.swapchain_id = swapchainId;
		detailWindow.surface_id = surfaceId;
		detailWindow.physical_device_id = physicalDeviceId;
		detailWindow.logical_device_id = logicalDeviceId;

		detail_window_[newId] = detailWindow;
		valid_detail_window_.push_back(detailWindow);

		return newId;
	}

	void ManagerWindow::RemoveWindow(const GraphicsWindowId& windowId)
	{
		for (size_t i = 0; i < valid_detail_window_.size(); i++)
		{
			if (valid_detail_window_[i].id_window == detail_window_[windowId].id_window)
			{
				utils::FastErase(valid_detail_window_, i);
				detail_window_.erase(windowId);

				return;
			}
		}
		LOGEXC(std::runtime_error, "[ManagerWindow::RemoveWindow] cant find window with this id");
	}

	[[nodiscard]] size_t ManagerWindow::GetNumWindow()
	{
		return detail_window_.size();
	}

	const std::vector<DetailWindow>& ManagerWindow::GetValidDetailWindows() const noexcept
	{
		return valid_detail_window_;
	}

	SwapchainId ManagerWindow::GetSwapchainId(const GraphicsWindowId& windowId) const
	{
		return detail_window_.at(windowId).swapchain_id;
	}

	SurfaceId ManagerWindow::GetSurfaceId(const GraphicsWindowId& windowId) const
	{
		return detail_window_.at(windowId).surface_id;
	}

	PhysicalDeviceId ManagerWindow::GetPhysicalDeviceId(const GraphicsWindowId& windowId) const
	{
		return detail_window_.at(windowId).physical_device_id;
	}

	LogicalDeviceId ManagerWindow::GetLogicalDeviceId(const GraphicsWindowId& windowId) const
	{
		auto find = detail_window_.find(windowId);
		if (find != detail_window_.end())
		{
			return find->second.logical_device_id;
		}
		LOG(Loglvl::warning, "[ManagerWindow::GetLogicalDeviceId] dont have window with id", windowId.GetId());
		return LogicalDeviceId();
	}

	std::shared_ptr<image::InterfaceImage> ManagerWindow::GetWindowPixelData(const GraphicsWindowId& windowId, bool alpha, FrameBufferId frameBufferId)
	{
		std::shared_ptr<image::InterfaceImageObject> srcImageData;
		TextureId textureId = getFrameBufferPixelData(windowId, srcImageData, frameBufferId);

		auto managerTextures = VulkanManagerTextures::Get();

		size_t strideBytes;
		const uint8_t* srcPixels = managerTextures->GetTextureData(textureId, &strideBytes);
		if (alpha)
		{
			srcImageData->pixels = new uint8_t[srcImageData->GetSize()];
			size_t pixelIndex = 0;
			for (uint32_t y = 0; y < static_cast<uint32_t>(srcImageData->GetTexHeight()); y++)
			{
				const uint8_t* row = srcPixels;
				for (uint32_t x = 0; x < static_cast<uint32_t>(srcImageData->GetTexWidth()); x++)
				{
					((uint8_t*)srcImageData->pixels)[pixelIndex++] = *(row++);
					((uint8_t*)srcImageData->pixels)[pixelIndex++] = *(row++);
					((uint8_t*)srcImageData->pixels)[pixelIndex++] = *(row++);
					((uint8_t*)srcImageData->pixels)[pixelIndex++] = *(row++);
				}
				srcPixels += strideBytes;
			}
		}
		else
		{
			srcImageData->format = description::Format::R8G8B8_UNORM;
			srcImageData->tex_channels = 3;
			srcImageData->size = (uint64_t)srcImageData->GetTexWidth() * (uint64_t)srcImageData->GetTexHeight() * (uint64_t)srcImageData->GetTexChannels();
			uint8_t* dstPixels = new uint8_t[srcImageData->GetSize()];
			srcImageData->pixels = dstPixels;
			size_t pixelIndex = 0;
			for (uint32_t y = 0; y < static_cast<uint32_t>(srcImageData->GetTexHeight()); y++)
			{
				const uint8_t* row = srcPixels;
				for (uint32_t x = 0; x < static_cast<uint32_t>(srcImageData->GetTexWidth()); x++)
				{
					((uint8_t*)srcImageData->pixels)[pixelIndex++] = *(row++);
					((uint8_t*)srcImageData->pixels)[pixelIndex++] = *(row++);
					((uint8_t*)srcImageData->pixels)[pixelIndex++] = *(row++);

					row++;
				}
				srcPixels += strideBytes;
			}
		}

		managerTextures->DeleteTexture(textureId);

		return srcImageData;
	}

	TextureId ManagerWindow::getFrameBufferPixelData(
		const GraphicsWindowId& windowId,
		std::shared_ptr<image::InterfaceImageObject>& imageSourceData,
		FrameBufferId frameBufferId)
	{
		if (!frameBufferId.IsValid())
		{
			return GeneratorId::GenerateInvalidId<TextureId>();
		}

		auto managerTextures = VulkanManagerTextures::Get();
		auto managerSwapChain = ManagerSwapchain::Get();

		const SwapchainId swapchainId = GetSwapchainId(windowId);
		auto fence = managerSwapChain->GetFence(swapchainId, managerSwapChain->GetCurrentFrameIndex(swapchainId))->fence;
		auto device = ManagerDevice::Get()->GetLogicalDevice(GetLogicalDeviceId(windowId))->device;

		vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

		imageSourceData = std::make_shared<image::InterfaceImageObject>();
		imageSourceData->format = description::Format::R8G8B8A8_UNORM;
		imageSourceData->usage = description::TextureUsage::TEXTURE;
		imageSourceData->memory_access = description::MemoryAccess::CPU;
		imageSourceData->is_mipmaps_enabled = false;
		const int channels = 4;
		imageSourceData->tex_channels = channels;

		const uint32_t index = managerSwapChain->GetCurrentImageIndex(swapchainId);

		glm::ivec2 size;
		VkImage srcImage;

		auto vulkanManagerFrameBuffer = VulkanManagerFrameBuffer::Get();
		auto windowFrameBufferId = vulkanManagerFrameBuffer->GetFrameBufferIdBySwapchainId(swapchainId);
		if (windowFrameBufferId == frameBufferId && managerSwapChain->GetSwapchain(swapchainId))
		{
			size = managerSwapChain->GetSize(swapchainId);
			srcImage = managerSwapChain->GetImage(swapchainId, index);
		}
		else
		{
			const TextureId colorAttachmentId = vulkanManagerFrameBuffer->GetFrameBufferAttachment(frameBufferId, index, 0);
			srcImage = managerTextures->GetImageData(colorAttachmentId)->image;
			size = managerTextures->GetTextureSize(colorAttachmentId);
		}

		imageSourceData->tex_width = size.x;
		imageSourceData->tex_height = size.y;
		imageSourceData->size = (uint64_t)size.x * (uint64_t)size.y * (uint64_t)channels;

		TextureId dstId = managerTextures->CreateTexture(windowId, imageSourceData);

		managerTextures->Copy(
			srcImage,
			managerTextures->GetImageData(dstId)->image,
			size,
			size,
			GetLogicalDeviceId(windowId),
			GetPhysicalDeviceId(windowId));

		return dstId;
	}
}
