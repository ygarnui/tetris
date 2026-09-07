#include "format.h"

namespace description
{
	std::unordered_map<description::Format, description::SupportedFormats::FormatInfo> description::SupportedFormats::formats =
	{
		{description::Format::R8_UNORM, {1, 1}},
		{description::Format::R8_SRGB, {1, 1}},
		{description::Format::R8G8_UNORM, {2, 1}},
		{description::Format::R8G8B8_UNORM, {3, 1}},
		{description::Format::R8G8B8A8_UNORM, {4, 1}},
		{description::Format::R32_SFLOAT, {1, 4}},
	};
}
