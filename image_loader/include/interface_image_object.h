#pragma once

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "../stb_image.h"

#include "interface_image.h"

#include <format.h>
#include <logger_instance.h>
#include <vector>

namespace image
{
    class InterfaceImageObject : public InterfaceImage
    {
    public:
        void* pixels = nullptr;
        int tex_width = 0;
        int tex_height = 0;
        int tex_channels = 0;
        int tex_channel_size = 0;
        uint64_t size = 0;
        description::Format format = description::Format::R8G8B8A8_UNORM;
        description::TextureUsage usage = description::TextureUsage::TEXTURE;
        description::MemoryAccess memory_access = description::MemoryAccess::GPU;
        SamplerCreateInfo sampler = {};
        bool is_mipmaps_enabled = true;

        InterfaceImageObject() = default;

        InterfaceImageObject(int image_width, int image_height, description::Format image_format)
        {
            tex_width = image_width;
            tex_height = image_height;

            auto formatInfoOpt = description::SupportedFormats::GetFormatInfo(image_format);
            if (formatInfoOpt.has_value())
            {
                auto formatInfo = formatInfoOpt.value();
                format = image_format;
                tex_channels = formatInfo.channels_num;
                tex_channel_size = formatInfo.channel_size_in_bytes;

                size = tex_width * tex_height * tex_channels * tex_channel_size;
                pixels = new char[size];
                memset(pixels, 0, size);
                return;
            }

            LOGEXC(std::invalid_argument, "[InterfaceImageObject::InterfaceImageObject] Can't find info about this format. Please add format the info to SupportedFormats class");
        }

        template<typename T>
        InterfaceImageObject(int image_width, int image_height, description::Format image_format, const std::vector<T>& image_pixels) : InterfaceImageObject(image_width, image_height, image_format)
        {
            memcpy(pixels, image_pixels.data(), size);
        }

        virtual ~InterfaceImageObject()
        {
            if (pixels)
            {
                stbi_image_free(pixels);
                pixels = nullptr;
            }
        }

        [[nodiscard]] virtual const void* GetPixels()
        {
            return pixels;
        }

        [[nodiscard]] virtual int GetTexWidth()
        {
            return tex_width;
        }

        [[nodiscard]] virtual int GetTexHeight()
        {
            return tex_height;
        }

        [[nodiscard]] virtual int GetTexChannels()
        {
            return tex_channels;
        }

        [[nodiscard]] virtual uint64_t GetSize()
        {
            return size;
        }

        [[nodiscard]] virtual description::Format GetFormat()
        {
            return format;
        }

        [[nodiscard]] virtual description::TextureUsage GetUsage()
        {
            return usage;
        }

        [[nodiscard]] virtual description::MemoryAccess GetMemoryAccess()
        {
            return memory_access;
        }

        [[nodiscard]] virtual SamplerCreateInfo GetSampler()
        {
            return sampler;
        }

        [[nodiscard]] virtual bool IsMipmapsEnabled()
        {
            return is_mipmaps_enabled;
        }
    };
}
