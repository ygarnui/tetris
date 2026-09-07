#pragma once

#include "interface_image.h"
#include "interface_image_object.h"
#include "manager_base.h"

#include <graphics_id.h>

#include <vector>
#include <map>
#include <memory>

namespace render
{
	struct DetailWindow
	{
		GraphicsWindowId id_window;
		// unique for each window
		SwapchainId swapchain_id;
		SurfaceId surface_id;

		// they can be the same for different windows
		PhysicalDeviceId physical_device_id;
		LogicalDeviceId logical_device_id;
	};

	class ManagerWindow : public ManagerBase
	{
	public:
		static std::shared_ptr<ManagerWindow>& Get();

		~ManagerWindow();

		ManagerWindow(const ManagerWindow&) = delete;
		ManagerWindow(ManagerWindow&&) = delete;

		ManagerWindow& operator= (const ManagerWindow&) = delete;
		ManagerWindow& operator= (ManagerWindow&&) = delete;

		[[nodiscard]] GraphicsWindowId AddWindow(
			const SwapchainId& swapchainId,
			const SurfaceId& surfaceId,
			const PhysicalDeviceId& physicalDeviceId,
			const LogicalDeviceId& logicalDeviceId);

		void RemoveWindow(const GraphicsWindowId& windowId);

		[[nodiscard]] size_t GetNumWindow();

		[[nodiscard]] const std::vector<DetailWindow>& GetValidDetailWindows() const noexcept;


		[[nodiscard]] SwapchainId GetSwapchainId(const GraphicsWindowId& windowId) const;
		[[nodiscard]] SurfaceId GetSurfaceId(const GraphicsWindowId& windowId) const;
		[[nodiscard]] PhysicalDeviceId GetPhysicalDeviceId(const GraphicsWindowId& windowId) const;
		[[nodiscard]] LogicalDeviceId GetLogicalDeviceId(const GraphicsWindowId& windowId) const;
		[[nodiscard]] std::shared_ptr<image::InterfaceImage> GetWindowPixelData(const GraphicsWindowId& windowId, bool alpha, FrameBufferId frameBufferId);

	private:
		ManagerWindow() = default;

		/*!
		\brief Transfer window frame buffer texture into cpu memory allocated texture.
		\param imageSourceData is the create info that the texture was created with.
		\param frameBufferId is framebuffer where the data comes from
		\return the texture id, which should be manually deleted.
		*/
		TextureId getFrameBufferPixelData(
			const GraphicsWindowId& windowId,
			std::shared_ptr<image::InterfaceImageObject>& imageSourceData,
			FrameBufferId frameBufferId);

		std::map<GraphicsWindowId, DetailWindow> detail_window_;
		std::vector<DetailWindow> valid_detail_window_;
	};
}
