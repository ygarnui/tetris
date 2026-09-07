#pragma once

#include "struct_data.h"
#include "device_property.h"
#include "manager_surface.h"
#include "manager_device.h"
#include "manager_base.h"

#include <vector>
#include <glm/ext.hpp>

namespace render
{
	struct DetailSwapchain
	{
		SurfaceId surface_id;
		LogicalDeviceId logical_device_id;
		PhysicalDeviceId physical_device_id;
		VkExtent2D current_extent = { 0, 0 };
		VkExtent2D source_extent = { 0, 0 };
		float ratio = 1.0f;
		uint32_t num_images = 0;
		uint32_t num_frames_in_flight = 0;
	};

	class ManagerSwapchain : public ManagerBase
	{
	public:
		static std::shared_ptr<ManagerSwapchain>& Get();

		~ManagerSwapchain();

		ManagerSwapchain(const ManagerSwapchain&) = delete;
		ManagerSwapchain(ManagerSwapchain&&) = delete;

		ManagerSwapchain& operator= (const ManagerSwapchain&) = delete;
		ManagerSwapchain& operator= (ManagerSwapchain&&) = delete;

		[[nodiscard]] SwapchainId AddSwapchain(
			const SurfaceId& surfaceId,
			const LogicalDeviceId& logicalDeviceId,
			const PhysicalDeviceId& physicalDeviceId,
			uint32_t width,
			uint32_t height,
			float windowRatio);

		[[nodiscard]] SwapchainId AddSwapchain(
			const LogicalDeviceId& logicalDeviceId,
			const PhysicalDeviceId& physicalDeviceId,
			const VkExtent2D& extent,
			const uint32_t numImages);

		[[nodiscard]] std::shared_ptr<DataSwapchain> GetSwapchain(const SwapchainId& swapchainId) const;
		[[nodiscard]] const std::vector<std::shared_ptr<DataImageView>>& GetImageViewData(const SwapchainId& swapchainId) const;
		[[nodiscard]] VkImage GetImage(const SwapchainId& swapchainId, const size_t index) const;
		[[nodiscard]] const uint32_t GetNumImages(const SwapchainId& swapchainId) const;
		[[nodiscard]] const uint32_t GetNumFramesInFlight(const SwapchainId& swapchainId) const;

		[[nodiscard]] std::shared_ptr<DataFence> GetFence(const SwapchainId& swapchainId, const uint32_t index) const;

		[[nodiscard]] std::shared_ptr<DataSemaphore> GetAvailableSemaphore(const SwapchainId& swapchainId, const uint32_t index) const;

		[[nodiscard]] std::shared_ptr<DataSemaphore> GetFinishedSemaphore(const SwapchainId& swapchainId, const uint32_t index) const;

		void SetRatio(const SwapchainId& swapchainId, float ratio);

		void Resize(const SwapchainId& swapchainId, const uint32_t width, const uint32_t height);

		void DeleteSwapchain(SwapchainId& swapchainId);

		void RecreateSwapchain(const SwapchainId& swapchainId);

		[[nodiscard]] const float GetAspect(const SwapchainId& swapchainId) const;
		[[nodiscard]] const glm::ivec2 GetSize(const SwapchainId& swapchainId) const;
		[[nodiscard]] const VkExtent2D GetExtent(const SwapchainId& swapchainId) const;
		[[nodiscard]] const float GetRatio(const SwapchainId& swapchainId) const;

		/*!
		\brief For resource acquisition aka command buffers, textures, buffers, etc.
		*/
		[[nodiscard]] const uint32_t GetCurrentImageIndex(const SwapchainId& swapchainId) const;

		/*!
		\brief For frame sync primitives acquisition aka fences, semaphores.
		*/
		[[nodiscard]] const uint32_t GetCurrentFrameIndex(const SwapchainId& swapchainId) const;
		
		const void SetCurrentImageIndex(const SwapchainId& swapchainId, uint32_t index);
		const void SetCurrentFrameIndex(const SwapchainId& swapchainId, uint32_t index);

	private:
		struct Swapchain
		{
			DetailSwapchain swapchain_detail_;
			std::shared_ptr<DataSwapchain> swapchain_;
			std::vector<VkImage> swapchain_images_;
			std::vector<std::shared_ptr<DataImageView>> image_views_;
			std::vector<std::shared_ptr<DataFence>> fences_;
			std::vector<std::shared_ptr<DataSemaphore>> available_semaphores_;
			std::vector<std::shared_ptr<DataSemaphore>> finished_semaphores_;

			/*!
			\brief For resource acquisition aka command buffers, textures, buffers, etc.
			*/
			uint32_t current_image_index_ = 0;

			/*!
			\brief For frame sync primitives acquisition aka fences, semaphores.
			*/
			uint32_t current_frame_index_ = 0;
		};

		ManagerSwapchain();

		size_t next_id_ = 0;

		Vector<Swapchain, SwapchainId> swapchain_data_;

		std::shared_ptr<ManagerSurface> manager_surface_;
		std::shared_ptr<ManagerDevice> manager_device_;
	};
}
