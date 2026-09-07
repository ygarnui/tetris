#include "creator_texture_sampler.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{
	std::shared_ptr<DataSampler> CreatorTextureSampler::CreateTextureSampler(
		std::shared_ptr<DataDevice> device,
		const image::SamplerCreateInfo& createInfo)
	{
		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = ConvertFilter(createInfo.filter);
		samplerCreateInfo.minFilter = samplerCreateInfo.magFilter;

		samplerCreateInfo.addressModeU = ConvertAddressMode(createInfo.addressMode);
		samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
		samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;

		samplerCreateInfo.anisotropyEnable = VK_TRUE;
		samplerCreateInfo.maxAnisotropy = createInfo.maxAnisotropy;

		samplerCreateInfo.borderColor = ConvertBorderColor(createInfo.borderColor);
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerCreateInfo.mipmapMode = ConvertMipmapMode(createInfo.mipmapMode);
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;

		std::shared_ptr<DataSampler> sampler(
			new DataSampler{
				{},
				device
			},
			[](DataSampler* p)
			{
				vkDestroySampler(p->device->device, p->sampler, nullptr);
				delete p;
			}
		);

		const VkResult result = vkCreateSampler(device->device, &samplerCreateInfo, nullptr, &sampler->sampler);
		if (result != VK_SUCCESS)
		{
			LOGEXC(std::runtime_error, "Failed to create texture sampler!");
		}

		return sampler;
	}

	VkSamplerAddressMode CreatorTextureSampler::ConvertAddressMode(image::SamplerCreateInfo::AddressMode addressMode)
	{
		switch (addressMode)
		{
		case image::SamplerCreateInfo::AddressMode::REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case image::SamplerCreateInfo::AddressMode::MIRRORED_REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case image::SamplerCreateInfo::AddressMode::CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case image::SamplerCreateInfo::AddressMode::CLAMP_TO_BORDER:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case image::SamplerCreateInfo::AddressMode::MIRROR_CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		default:
			break;
		}

		LOGEXC(std::runtime_error, "Failed to convert address mode!");
		return {};
	}

	image::SamplerCreateInfo::AddressMode CreatorTextureSampler::ConvertAddressMode(VkSamplerAddressMode addressMode)
	{
		switch (addressMode)
		{
		case VK_SAMPLER_ADDRESS_MODE_REPEAT:
			return image::SamplerCreateInfo::AddressMode::REPEAT;
		case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:
			return image::SamplerCreateInfo::AddressMode::MIRRORED_REPEAT;
		case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
			return image::SamplerCreateInfo::AddressMode::CLAMP_TO_EDGE;
		case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
			return image::SamplerCreateInfo::AddressMode::CLAMP_TO_BORDER;
		case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
			return image::SamplerCreateInfo::AddressMode::MIRROR_CLAMP_TO_EDGE;
		default:
			break;
		}

		LOGEXC(std::runtime_error, "Failed to convert address mode!");
		return {};
	}

	VkFilter CreatorTextureSampler::ConvertFilter(image::SamplerCreateInfo::Filter filter)
	{
		switch (filter)
		{
		case image::SamplerCreateInfo::Filter::NEAREST:
			return VK_FILTER_NEAREST;
		case image::SamplerCreateInfo::Filter::LINEAR:
			return VK_FILTER_LINEAR;
		default:
			break;
		}

		LOGEXC(std::runtime_error, "Failed to convert filter!");
		return {};
	}

	image::SamplerCreateInfo::Filter CreatorTextureSampler::ConvertFilter(VkFilter filter)
	{
		switch (filter)
		{
		case VK_FILTER_NEAREST:
			return image::SamplerCreateInfo::Filter::NEAREST;
		case VK_FILTER_LINEAR:
			return image::SamplerCreateInfo::Filter::LINEAR;
		default:
			break;
		}

		LOGEXC(std::runtime_error, "Failed to convert filter!");
		return {};
	}

	VkSamplerMipmapMode CreatorTextureSampler::ConvertMipmapMode(image::SamplerCreateInfo::MipmapMode mipmapMode)
	{
		switch (mipmapMode)
		{
		case image::SamplerCreateInfo::MipmapMode::NEAREST:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case image::SamplerCreateInfo::MipmapMode::LINEAR:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		default:
			break;
		}

		LOGEXC(std::runtime_error, "Failed to convert mipmap mode!");
		return {};
	}

	image::SamplerCreateInfo::MipmapMode CreatorTextureSampler::ConvertMipmapMode(VkSamplerMipmapMode mipmapMode)
	{
		switch (mipmapMode)
		{
		case VK_SAMPLER_MIPMAP_MODE_NEAREST:
			return image::SamplerCreateInfo::MipmapMode::NEAREST;
		case VK_SAMPLER_MIPMAP_MODE_LINEAR:
			return image::SamplerCreateInfo::MipmapMode::LINEAR;
		default:
			break;
		}

		LOGEXC(std::runtime_error, "Failed to convert mipmap mode!");
		return {};
	}

	VkBorderColor CreatorTextureSampler::ConvertBorderColor(image::SamplerCreateInfo::BorderColor borderColor)
	{
		switch (borderColor)
		{
		case image::SamplerCreateInfo::BorderColor::FLOAT_TRANSPARENT_BLACK:
			return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		case image::SamplerCreateInfo::BorderColor::INT_TRANSPARENT_BLACK:
			return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		case image::SamplerCreateInfo::BorderColor::FLOAT_OPAQUE_BLACK:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case image::SamplerCreateInfo::BorderColor::INT_OPAQUE_BLACK:
			return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		case image::SamplerCreateInfo::BorderColor::FLOAT_OPAQUE_WHITE:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		case image::SamplerCreateInfo::BorderColor::INT_OPAQUE_WHITE:
			return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		default:
			break;
		}

		LOGEXC(std::runtime_error, "Failed to convert border color!");
		return {};
	}

	image::SamplerCreateInfo::BorderColor CreatorTextureSampler::ConvertBorderColor(VkBorderColor borderColor)
	{
		switch (borderColor)
		{
		case VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:
			return image::SamplerCreateInfo::BorderColor::FLOAT_TRANSPARENT_BLACK;
		case VK_BORDER_COLOR_INT_TRANSPARENT_BLACK:
			return image::SamplerCreateInfo::BorderColor::INT_TRANSPARENT_BLACK;
		case VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK:
			return image::SamplerCreateInfo::BorderColor::FLOAT_OPAQUE_BLACK;
		case VK_BORDER_COLOR_INT_OPAQUE_BLACK:
			return image::SamplerCreateInfo::BorderColor::INT_OPAQUE_BLACK;
		case VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE:
			return image::SamplerCreateInfo::BorderColor::FLOAT_OPAQUE_WHITE;
		case VK_BORDER_COLOR_INT_OPAQUE_WHITE:
			return image::SamplerCreateInfo::BorderColor::INT_OPAQUE_WHITE;
		default:
			break;
		}

		LOGEXC(std::runtime_error, "Failed to convert border color!");
		return {};
	}
}
