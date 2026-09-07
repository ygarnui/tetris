#pragma once

#include <string>

namespace general
{
	class InterfaceManagerAsset
	{
	public:
		virtual const std::string& GetAssetPath() const = 0;
	};
}
