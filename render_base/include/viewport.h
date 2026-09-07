#pragma once

#include <cfloat>

namespace render
{
    class Viewport {
    public:
        float x = FLT_MAX;
        float y = FLT_MAX;
        float width = FLT_MAX;
        float height = FLT_MAX;
        float minDepth = FLT_MAX;
        float maxDepth = FLT_MAX;

        bool IsValid()
        {
            return
                x < FLT_MAX &&
                y < FLT_MAX &&
                width < FLT_MAX &&
                height < FLT_MAX &&
                minDepth < maxDepth &&
                0.0f <= minDepth && minDepth <= 1.0f &&
                0.0f <= maxDepth && maxDepth <= 1.0f;
        }

        bool operator ==(const Viewport& viewport)
        {
            return
                x == viewport.x &&
                y == viewport.y &&
                width == viewport.width &&
                height == viewport.height &&
                minDepth == viewport.minDepth &&
                maxDepth == viewport.maxDepth;
        }
    };

    template <typename V1, typename V2>
    bool EquallyViewport(const V1& v1, const V2& v2)
    {
        return
            v1.height == v2.height &&
            v1.maxDepth == v2.maxDepth &&
            v1.minDepth == v2.minDepth &&
            v1.width == v2.width &&
            v1.x == v2.x &&
            v1.y == v2.y;
    }
}
