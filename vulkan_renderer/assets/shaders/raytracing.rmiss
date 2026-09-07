#version 460
#extension GL_EXT_ray_tracing : require

#include "globals/raytracing.h"

layout(location = 0) rayPayloadInEXT RtPayload payload;

/*!
The same miss shader serves primary and shadow rays. Clearing the hit flag is all either
one needs: the ray generation shader reads it as "the ray escaped the scene", which for a
shadow ray means the light is visible, and for a primary ray means the sky was reached.
*/
void main()
{
	payload.hit = 0;
}
