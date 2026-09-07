#pragma once

#include <stdint.h>

#include "buffer_description.h"

namespace image
{
	class InterfaceImage
	{
	public:

		[[nodiscard]] virtual const void* GetPixels() = 0;
		[[nodiscard]] virtual int GetTexWidth() = 0;
		[[nodiscard]] virtual int GetTexHeight() = 0;
		[[nodiscard]] virtual int GetTexChannels() = 0;
		[[nodiscard]] virtual uint64_t GetSize() = 0;
		[[nodiscard]] virtual description::Format GetFormat() = 0;
		[[nodiscard]] virtual description::TextureUsage GetUsage() = 0;
		[[nodiscard]] virtual description::MemoryAccess GetMemoryAccess() = 0;
		[[nodiscard]] virtual SamplerCreateInfo GetSampler() = 0;
		[[nodiscard]] virtual bool IsMipmapsEnabled() = 0;
	};
}
