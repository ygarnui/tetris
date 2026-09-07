#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

#include "globals/raytracing.h"

layout(scalar, set = 0, binding = RT_BINDING_VERTICES) readonly buffer VertexBuffer
{
	RtVertex vertices[];
};

layout(scalar, set = 0, binding = RT_BINDING_INDICES) readonly buffer IndexBuffer
{
	uint indices[];
};

layout(scalar, set = 0, binding = RT_BINDING_INSTANCES) readonly buffer InstanceBuffer
{
	RtInstance instances[];
};

layout(location = 0) rayPayloadInEXT RtPayload payload;

hitAttributeEXT vec2 barycentrics;

/*!
Reports the hit back to the ray generation shader, which does all of the shading. Every
instance points at its own range of the shared vertex and index buffers, so the same box
geometry can be reused by every object in the scene.
*/
void main()
{
	const RtInstance instance = instances[gl_InstanceCustomIndexEXT];

	const uint indexOffset = instance.first_index + 3 * gl_PrimitiveID;

	const uint i0 = instance.first_vertex + indices[indexOffset + 0];
	const uint i1 = instance.first_vertex + indices[indexOffset + 1];
	const uint i2 = instance.first_vertex + indices[indexOffset + 2];

	const vec3 weights = vec3(
		1.0 - barycentrics.x - barycentrics.y,
		barycentrics.x,
		barycentrics.y);

	const vec3 objectNormal =
		vertices[i0].normal * weights.x +
		vertices[i1].normal * weights.y +
		vertices[i2].normal * weights.z;

	// gl_ObjectToWorldEXT is a 4x3 matrix; the scene only uses rotation, uniform scale and
	// translation, so its upper 3x3 transforms normals correctly after normalisation.
	const vec3 worldNormal = normalize(mat3(gl_ObjectToWorldEXT) * objectNormal);

	payload.position = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	// Always report the side of the surface the ray came from.
	payload.normal = dot(worldNormal, gl_WorldRayDirectionEXT) < 0.0 ? worldNormal : -worldNormal;
	payload.albedo = instance.albedo_reflectivity.rgb;
	payload.reflectivity = instance.albedo_reflectivity.a;
	payload.hit = 1;
}
