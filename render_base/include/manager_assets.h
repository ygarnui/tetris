#pragma once

#include <memory>

namespace render
{
	template <typename InterfaceManagerAsset>
	class ManagerAssets
	{
	public:
		ManagerAssets(const ManagerAssets&) = delete;
		ManagerAssets(ManagerAssets&&) = delete;

		ManagerAssets& operator= (const ManagerAssets&) = delete;
		ManagerAssets& operator= (ManagerAssets&&) = delete;

		void Init(std::shared_ptr<InterfaceManagerAsset> interfaceManagerAsset)
		{
			interface_manager_asset_ = interfaceManagerAsset;
		}

	protected:
		ManagerAssets() = default;

		std::shared_ptr<InterfaceManagerAsset> getInterfaceManagerAsset() const
		{
			return interface_manager_asset_;
		}

		std::shared_ptr<InterfaceManagerAsset> interface_manager_asset_;
	};
}
