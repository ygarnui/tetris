#pragma once

#include <vulkan/vulkan_core.h>
#include <logger_instance.h>

#include <iostream>
#include <type_traits>


namespace render
{
	template<typename T>
	bool CheckType(const T& curType, const T type, std::string message = "") {
		if (curType != type)
		{
			if (message.size() != 0)
			{
				LOG(Loglvl::info, "[render::CheckType]", message);
			}
			return false;
		}
		return true;
	}
}

