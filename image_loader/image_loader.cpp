#include "include/image_loader.h"
#include "stb_image_write.h"

#include <logger_instance.h>

namespace image
{
    std::shared_ptr<InterfaceImage> ImageLoader::CreateTextureImage(
        const description::ImportImageDescription& imageInfo,
        bool forceAlpha)
    {
        std::shared_ptr<InterfaceImageObject> image = std::make_shared<InterfaceImageObject>();

        int reqComp = STBI_default;
        if (forceAlpha)
        {
            reqComp = STBI_rgb_alpha;
        }

        image->pixels = stbi_load(
            imageInfo.filename.data(),
            &image->tex_width,
            &image->tex_height,
            &image->tex_channels,
            reqComp);

        if (forceAlpha)
        {
            image->tex_channels = STBI_rgb_alpha;
        }

        image->size = static_cast<uint64_t>(image->GetTexWidth()) *
            static_cast<uint64_t>(image->GetTexHeight()) *
            static_cast<uint64_t>(image->GetTexChannels());
        image->sampler.addressMode = imageInfo.address_mode;

        if (!image->GetPixels())
        {
            LOGEXC(std::runtime_error, "[ImageLoader::CreateTextureImage] failed to load texture image!");
        }

        return image;
    }
}
