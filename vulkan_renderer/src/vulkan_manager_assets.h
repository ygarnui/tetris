#pragma once

#include <interface_manager_asset_render.h>
#include <manager_assets.h>

namespace render
{
	class ManagerAssetsVulkan : public ManagerAssets<general::InterfaceManagerAssetRender>
	{
	public:
		static std::shared_ptr<ManagerAssetsVulkan> Get()
		{
			static auto manager = std::shared_ptr<ManagerAssetsVulkan>(new ManagerAssetsVulkan());
			return manager;
		}

		static std::shared_ptr<general::InterfaceManagerAssetRender> GetInterface()
		{
			return ManagerAssetsVulkan::Get()->getInterfaceManagerAsset();
		}

	private:
		ManagerAssetsVulkan() = default;
	};
}
