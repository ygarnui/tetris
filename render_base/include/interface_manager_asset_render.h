#pragma once
#include "interface_manager_asset.h"

#include <string>

namespace general
{
	class InterfaceManagerAssetRender : public InterfaceManagerAsset
	{
	public:
		virtual const std::string& GetShaderCachePath() const = 0;
	};
}
