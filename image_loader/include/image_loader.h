#pragma once

#include "interface_image_object.h"

#include <stdexcept>
#include <memory>

namespace image
{
    class ImageLoader
    {
    public:
        [[nodiscard]] static std::shared_ptr<InterfaceImage> CreateTextureImage(
            const description::ImportImageDescription& imageInfo,
            bool forceAlpha);
    };
}
