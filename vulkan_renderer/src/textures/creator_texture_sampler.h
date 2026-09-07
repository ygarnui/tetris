#pragma once

#include <interface_image_object.h>

#include "../struct_data.h"

namespace render
{
	class CreatorTextureSampler
	{
	public:
		[[nodiscard]] static std::shared_ptr<DataSampler> CreateTextureSampler(
			std::shared_ptr<DataDevice> device,
			const image::SamplerCreateInfo& createInfo);

		[[nodiscard]]  static VkSamplerAddressMode ConvertAddressMode(image::SamplerCreateInfo::AddressMode addressMode);

		[[nodiscard]]  static image::SamplerCreateInfo::AddressMode ConvertAddressMode(VkSamplerAddressMode addressMode);

		[[nodiscard]]  static VkFilter ConvertFilter(image::SamplerCreateInfo::Filter filter);

		[[nodiscard]]  static image::SamplerCreateInfo::Filter ConvertFilter(VkFilter filter);

		[[nodiscard]]  static VkSamplerMipmapMode ConvertMipmapMode(image::SamplerCreateInfo::MipmapMode mipmapMode);

		[[nodiscard]]  static image::SamplerCreateInfo::MipmapMode ConvertMipmapMode(VkSamplerMipmapMode mipmapMode);

		[[nodiscard]]  static VkBorderColor ConvertBorderColor(image::SamplerCreateInfo::BorderColor borderColor);

		[[nodiscard]]  static image::SamplerCreateInfo::BorderColor ConvertBorderColor(VkBorderColor borderColor);
	};
}
