#include <unordered_map>
#include <string>
#include <logger_instance.h>

namespace image
{
	struct SamplerCreateInfo
	{
		enum class Filter
		{
			NEAREST = 0,
			LINEAR = 1
		};

		enum class AddressMode
		{
			REPEAT = 0,
			MIRRORED_REPEAT = 1,
			CLAMP_TO_EDGE = 2,
			CLAMP_TO_BORDER = 3,
			MIRROR_CLAMP_TO_EDGE = 4,
			COUNT
		};

		enum class MipmapMode
		{
			NEAREST = 0,
			LINEAR = 1
		};

		enum class BorderColor
		{
			FLOAT_TRANSPARENT_BLACK = 0,
			INT_TRANSPARENT_BLACK = 1,
			FLOAT_OPAQUE_BLACK = 2,
			INT_OPAQUE_BLACK = 3,
			FLOAT_OPAQUE_WHITE = 4,
			INT_OPAQUE_WHITE = 5
		};

		float maxAnisotropy = 16.0f;
		Filter filter = Filter::LINEAR;
		AddressMode addressMode = AddressMode::REPEAT;
		MipmapMode mipmapMode = MipmapMode::LINEAR;
		BorderColor borderColor = BorderColor::FLOAT_OPAQUE_BLACK;
	};

	class AddressModeMap
	{
	public:
		static const std::unordered_map<std::string, SamplerCreateInfo::AddressMode>& GetMap()
		{
			if (addressModes.size() != static_cast<size_t>(SamplerCreateInfo::AddressMode::COUNT))
			{
				LOGEXC(std::runtime_error, "Size of AddressModeMap must be equal to SamplerCreateInfo::AddressMode::COUNT");
			}

			return addressModes;
		}

	private:
		static inline const std::unordered_map<std::string, SamplerCreateInfo::AddressMode> addressModes
		{
			{"REPEAT", SamplerCreateInfo::AddressMode::REPEAT},
			{"MIRRORED_REPEAT", SamplerCreateInfo::AddressMode::MIRRORED_REPEAT},
			{"CLAMP_TO_EDGE", SamplerCreateInfo::AddressMode::CLAMP_TO_EDGE},
			{"CLAMP_TO_BORDER", SamplerCreateInfo::AddressMode::CLAMP_TO_BORDER},
			{"MIRROR_CLAMP_TO_EDGE", SamplerCreateInfo::AddressMode::MIRROR_CLAMP_TO_EDGE},
		};
	};
}
