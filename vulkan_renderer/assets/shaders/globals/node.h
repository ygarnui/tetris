// Define extensions
#ifndef __cplusplus
#extension GL_EXT_scalar_block_layout : enable
#endif

#ifdef __cplusplus
#pragma once
namespace shaders
{
	using namespace glm;
#endif

    struct Node
    {
        vec4 color;
        float depth;
        uint next;
    };

    struct GeometryCount
    {
        uint count;
        uint maxNodeCount;
    };

#ifdef __cplusplus
}
#endif
