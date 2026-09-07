#include "manager_swapchain.h"

#include "creator_swapchain.h"
#include "creator_images.h"
#include "creator_image_views.h"
#include "creator_fence.h"
#include "creator_semaphore.h"
#include "vulkan_manager_frame_buffer.h"
#include "selector_swapchain_settings.h"
#include "textures/creator_image_buffer.h"

#include <guard_next_id.h>
#include <generator_id.h>
#include <logger_instance.h>

#include <stdexcept>

namespace render
{
	ManagerSwapchain::ManagerSwapchain()
	{
		manager_surface_ = ManagerSurface::Get();
		manager_device_ = ManagerDevice::Get();
	}

	std::shared_ptr<ManagerSwapchain>& ManagerSwapchain::Get()
	{
		static std::shared_ptr<ManagerSwapchain> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<ManagerSwapchain>(new ManagerSwapchain());
		}
		return manager;
	}

	ManagerSwapchain::~ManagerSwapchain()
	{
		LOG(Loglvl::debug, "[ManagerSwapchain::~ManagerSwapchain]");
	}

	SwapchainId ManagerSwapchain::AddSwapchain(
		const SurfaceId& surfaceId,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId,
		uint32_t width,
		uint32_t height,
		float windowRatio)
	{
		auto logicalDevice = manager_device_->GetLogicalDevice(logicalDeviceId);
		auto physicalDevice = manager_device_->GetPhysicalDevice(physicalDeviceId);
		auto surface = manager_surface_->GetSurface(surfaceId);
		auto swapChainSupport = manager_device_->GetSwapchainSupportDetailsData(physicalDeviceId, surfaceId);
		auto indices = manager_device_->GetPhysicalDeviceProperties(physicalDeviceId).queue_family_indices;

		auto newId = GeneratorId::GenerateUniqueId<SwapchainId>(next_id_);

		auto guardId = utils::GuardResize::MayBeResize(
			next_id_,
			[&](const Swapchain& swapchain) { return !swapchain.swapchain_detail_.surface_id.IsValid(); },
			swapchain_data_);

		auto extent = VkExtent2D(uint32_t(width * windowRatio), uint32_t(height * windowRatio));

		const VkExtent2D currentExtent = SelectorSwapchainSettings::ChooseSwapExtent(swapChainSupport.capabilities, extent);

		swapchain_data_[newId].swapchain_ = CreatorSwapchain::CreateSwapchain(
			swapChainSupport,
			logicalDevice,
			physicalDevice,
			surface,
			currentExtent,
			indices[0]);

		swapchain_data_[newId].swapchain_images_ = CreatorImages::CreateImages(logicalDevice, swapchain_data_[newId].swapchain_);

		DetailSwapchain detailSwapchain;

		detailSwapchain.surface_id = surfaceId;
		detailSwapchain.logical_device_id = logicalDeviceId;
		detailSwapchain.physical_device_id = physicalDeviceId;
		detailSwapchain.current_extent = currentExtent;
		detailSwapchain.source_extent = { width, height };
		detailSwapchain.ratio = windowRatio;
		detailSwapchain.num_images = static_cast<uint32_t>(swapchain_data_[newId].swapchain_images_.size());
		detailSwapchain.num_frames_in_flight = static_cast<uint32_t>(swapchain_data_[newId].swapchain_images_.size());

		swapchain_data_[newId].swapchain_detail_ = detailSwapchain;

		VkComponentMapping components = {
		  VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R,
		  VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G,
		  VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B,
		  VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A
		};

		swapchain_data_[newId].image_views_ = CreatorImageView::CreateImageViews(
			logicalDevice,
			swapchain_data_[newId].swapchain_images_,
			swapchain_data_[newId].swapchain_->format.format,
			VK_IMAGE_ASPECT_COLOR_BIT,
			components);

		bool supportMsaa = false;
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		uint32_t one = 1;
		for (size_t i = 1; i < 7; i++)
		{
			VkSampleCountFlags curFlag = one << i;
			if (deviceProperties.limits.framebufferColorSampleCounts & curFlag)
			{
				supportMsaa = true;
				break;
			}
		}

		swapchain_data_[newId].fences_.resize(swapchain_data_[newId].swapchain_detail_.num_frames_in_flight);
		swapchain_data_[newId].available_semaphores_.resize(swapchain_data_[newId].swapchain_detail_.num_frames_in_flight);
		swapchain_data_[newId].finished_semaphores_.resize(swapchain_data_[newId].swapchain_detail_.num_frames_in_flight);
		for (size_t i = 0; i < swapchain_data_[newId].swapchain_detail_.num_frames_in_flight; i++)
		{
			swapchain_data_[newId].fences_[i] = CreatorFence::CreateFence(logicalDevice);
			swapchain_data_[newId].available_semaphores_[i] = CreatorSemaphore::CreateSemaphore(logicalDevice);
			swapchain_data_[newId].finished_semaphores_[i] = CreatorSemaphore::CreateSemaphore(logicalDevice);
		}

		return newId;
	}

	SwapchainId ManagerSwapchain::AddSwapchain(
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId,
		const VkExtent2D& extent,
		const uint32_t numImages)
	{
		const auto logicalDevice = manager_device_->GetLogicalDevice(logicalDeviceId);

		const auto newId = GeneratorId::GenerateUniqueId<SwapchainId>(next_id_);

		auto guardId = utils::GuardResize::MayBeResize(
			next_id_,
			[&](const Swapchain& swapchain) { return swapchain.fences_.empty(); },
			swapchain_data_);

		DetailSwapchain detailSwapchain;
		detailSwapchain.surface_id = SurfaceId();
		detailSwapchain.logical_device_id = logicalDeviceId;
		detailSwapchain.physical_device_id = physicalDeviceId;
		detailSwapchain.current_extent = extent;
		detailSwapchain.source_extent = extent;
		detailSwapchain.ratio = 1.0f;
		detailSwapchain.num_images = numImages;
		detailSwapchain.num_frames_in_flight = numImages;

		auto& newSwapchain = swapchain_data_[newId].swapchain_detail_ = detailSwapchain;

		swapchain_data_[newId].fences_.resize(swapchain_data_[newId].swapchain_detail_.num_frames_in_flight);
		swapchain_data_[newId].available_semaphores_.resize(swapchain_data_[newId].swapchain_detail_.num_frames_in_flight);
		swapchain_data_[newId].finished_semaphores_.resize(swapchain_data_[newId].swapchain_detail_.num_frames_in_flight);
		for (size_t i = 0; i < swapchain_data_[newId].swapchain_detail_.num_frames_in_flight; i++)
		{
			swapchain_data_[newId].fences_[i] = CreatorFence::CreateFence(logicalDevice);
			swapchain_data_[newId].available_semaphores_[i] = CreatorSemaphore::CreateSemaphore(logicalDevice);
			swapchain_data_[newId].finished_semaphores_[i] = CreatorSemaphore::CreateSemaphore(logicalDevice);
		}

		return newId;
	}

	std::shared_ptr<DataSwapchain> ManagerSwapchain::GetSwapchain(const SwapchainId& swapchainId) const
	{
		return swapchain_data_[swapchainId].swapchain_;
	}

	const std::vector<std::shared_ptr<DataImageView>>& ManagerSwapchain::GetImageViewData(const SwapchainId& swapchainId) const
	{
		return swapchain_data_[swapchainId].image_views_;
	}

	VkImage ManagerSwapchain::GetImage(const SwapchainId& swapchainId, const size_t index) const
	{
		return swapchain_data_[swapchainId].swapchain_images_.at(index);
	}

	const uint32_t ManagerSwapchain::GetNumImages(const SwapchainId& swapchainId) const
	{
		return swapchain_data_[swapchainId].swapchain_detail_.num_images;
	}

	const uint32_t ManagerSwapchain::GetNumFramesInFlight(const SwapchainId& swapchainId) const
	{
		return swapchain_data_[swapchainId].swapchain_detail_.num_frames_in_flight;
	}

	std::shared_ptr<DataFence> ManagerSwapchain::GetFence(const SwapchainId& swapchainId, const uint32_t index) const
	{
		return swapchain_data_[swapchainId].fences_[index];
	}

	std::shared_ptr<DataSemaphore> ManagerSwapchain::GetAvailableSemaphore(const SwapchainId& swapchainId, const uint32_t index) const
	{
		return swapchain_data_[swapchainId].available_semaphores_[index];
	}

	std::shared_ptr<DataSemaphore> ManagerSwapchain::GetFinishedSemaphore(const SwapchainId& swapchainId, const uint32_t index) const
	{
		return swapchain_data_[swapchainId].finished_semaphores_[index];
	}

	void ManagerSwapchain::SetRatio(const SwapchainId& swapchainId, float ratio)
	{
		if (swapchain_data_[swapchainId].swapchain_detail_.ratio == ratio)
		{
			return;
		}

		swapchain_data_[swapchainId].swapchain_detail_.ratio = ratio;
		RecreateSwapchain(swapchainId);
	}

	void ManagerSwapchain::Resize(const SwapchainId& swapchainId, const uint32_t width, const uint32_t height)
	{
		swapchain_data_[swapchainId].swapchain_detail_.source_extent = { width, height };
	}

	void ManagerSwapchain::DeleteSwapchain(SwapchainId& swapchainId)
	{
		if (swapchain_data_[swapchainId].swapchain_.use_count() > 1)
		{
			LOGEXC(std::runtime_error, "[ManagerSwapchain::DeleteSwapchain] swapchain use_count > 1");
		}

		next_id_ = std::min(next_id_, swapchainId.GetId());
		swapchain_data_[swapchainId] = {};

		swapchainId = GeneratorId::GenerateUniqueId<SwapchainId>(-1);
	}

	void ManagerSwapchain::RecreateSwapchain(const SwapchainId& swapchainId)
	{
		auto& swapchain = swapchain_data_[swapchainId];
		auto& swapchainDetail = swapchain_data_[swapchainId].swapchain_detail_;
		auto logicalDevice = manager_device_->GetLogicalDevice(swapchainDetail.logical_device_id);
		auto physicalDevice = manager_device_->GetPhysicalDevice(swapchainDetail.physical_device_id);
		auto surface = manager_surface_->GetSurface(swapchainDetail.surface_id);
		auto indices = manager_device_->GetPhysicalDeviceProperties(swapchainDetail.physical_device_id).queue_family_indices;

		manager_device_->AddSurfaceCapabilities(swapchainDetail.physical_device_id, swapchainDetail.surface_id, surface);
		auto swapChainSupport = manager_device_->GetSwapchainSupportDetailsData(swapchainDetail.physical_device_id, swapchainDetail.surface_id);

		const auto ratio = swapchainDetail.ratio;
		auto extent = swapchainDetail.source_extent;
		extent = { uint32_t(extent.width * ratio), uint32_t(extent.height * ratio) };
		swapchainDetail.current_extent = SelectorSwapchainSettings::ChooseSwapExtent(swapChainSupport.capabilities, extent);

		swapchain.swapchain_ = nullptr;
		swapchain.swapchain_ = CreatorSwapchain::CreateSwapchain(
			swapChainSupport,
			logicalDevice,
			physicalDevice,
			surface,
			swapchainDetail.current_extent,
			indices[0]);

		swapchain.swapchain_images_ = CreatorImages::CreateImages(logicalDevice, swapchain.swapchain_);

		swapchain.swapchain_detail_.num_images = static_cast<uint32_t>(swapchain.swapchain_images_.size());

		VkComponentMapping components = {
			VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R,
			VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G,
			VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B,
			VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A
		};

		swapchain.image_views_ = CreatorImageView::CreateImageViews(
			logicalDevice,
			swapchain.swapchain_images_,
			swapchain.swapchain_->format.format,
			VK_IMAGE_ASPECT_COLOR_BIT,
			components);

		VulkanManagerFrameBuffer::Get()->ApplyResize();
	}

	const float ManagerSwapchain::GetAspect(const SwapchainId& swapchainId) const
	{
		const auto& curExtent = swapchain_data_[swapchainId].swapchain_detail_.current_extent;

		if (curExtent.height > 0)
		{
			return static_cast<float>(curExtent.width) / static_cast<float>(curExtent.height);
		}

		return 1.0f;
	}

	const glm::ivec2 ManagerSwapchain::GetSize(const SwapchainId& swapchainId) const
	{
		const auto& extent = swapchain_data_[swapchainId].swapchain_detail_.current_extent;
		return glm::ivec2(extent.width, extent.height);
	}

	const VkExtent2D ManagerSwapchain::GetExtent(const SwapchainId& swapchainId) const
	{
		return swapchain_data_[swapchainId].swapchain_detail_.current_extent;
	}

	const float ManagerSwapchain::GetRatio(const SwapchainId& swapchainId) const
	{
		return swapchain_data_[swapchainId].swapchain_detail_.ratio;
	}

	const uint32_t ManagerSwapchain::GetCurrentImageIndex(const SwapchainId& swapchainId) const
	{
		return swapchain_data_[swapchainId].current_image_index_;
	}

	const uint32_t ManagerSwapchain::GetCurrentFrameIndex(const SwapchainId& swapchainId) const
	{
		return swapchain_data_[swapchainId].current_frame_index_;
	}

	const void ManagerSwapchain::SetCurrentImageIndex(const SwapchainId& swapchainId, uint32_t index)
	{
		swapchain_data_[swapchainId].current_image_index_ = index;
	}

	const void ManagerSwapchain::SetCurrentFrameIndex(const SwapchainId& swapchainId, uint32_t index)
	{
		swapchain_data_[swapchainId].current_frame_index_ = index;
	}
}
