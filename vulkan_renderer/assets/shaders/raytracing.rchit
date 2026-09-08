#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require

#include "globals/raytracing.h"

layout(scalar, set = 0, binding = RT_BINDING_INSTANCES) readonly buffer InstanceBuffer
{
	RtInstance instances[];
};

// Indexed by RtInstance::texture_index; nonuniformEXT is required because that index
// varies per ray within the same shader invocation group (enabled on the device via
// VkPhysicalDeviceVulkan12Features::shaderSampledImageArrayNonUniformIndexing, already
// required by VK_KHR_acceleration_structure).
layout(set = 0, binding = RT_BINDING_TEXTURES) uniform sampler2D textures[RT_MAX_TEXTURES];

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

	const uint indexOffset = 3 * gl_PrimitiveID;
	
	const uint i0 = IndexBufferRef(instance.index_buffer_address).indices[indexOffset + 0];
	const uint i1 = IndexBufferRef(instance.index_buffer_address).indices[indexOffset + 1];
	const uint i2 = IndexBufferRef(instance.index_buffer_address).indices[indexOffset + 2];

	const vec3 weights = vec3(
		1.0 - barycentrics.x - barycentrics.y,
		barycentrics.x,
		barycentrics.y);

	const vec3 objectNormal =
		VertexBufferRef(instance.vertex_buffer_address).vertices[i0].normal * weights.x +
		VertexBufferRef(instance.vertex_buffer_address).vertices[i1].normal * weights.y +
		VertexBufferRef(instance.vertex_buffer_address).vertices[i2].normal * weights.z;

	const vec2 uv =
		VertexBufferRef(instance.vertex_buffer_address).vertices[i0].uv * weights.x +
		VertexBufferRef(instance.vertex_buffer_address).vertices[i1].uv * weights.y +
		VertexBufferRef(instance.vertex_buffer_address).vertices[i2].uv * weights.z;

	// gl_ObjectToWorldEXT is a 4x3 matrix; the scene only uses rotation, uniform scale and
	// translation, so its upper 3x3 transforms normals correctly after normalisation.
	const vec3 worldNormal = normalize(mat3(gl_ObjectToWorldEXT) * objectNormal);

	// A face with no texture interpolates the (-1, -1) sentinel from every one of its
	// corners unchanged, so this also correctly skips instances that do have a texture but
	// were hit on an untextured face (e.g. the sides of a button, not its top).
	const bool hasTextureHere = instance.texture_index != RT_NO_TEXTURE && uv.x >= 0.0 && uv.y >= 0.0;

	payload.position = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	// Always report the side of the surface the ray came from.
	payload.normal = dot(worldNormal, gl_WorldRayDirectionEXT) < 0.0 ? worldNormal : -worldNormal;
	payload.albedo = hasTextureHere
		? texture(textures[nonuniformEXT(instance.texture_index)], uv).rgb
		: instance.albedo_reflectivity.rgb;
	payload.reflectivity = instance.albedo_reflectivity.a;
	payload.hit = 1;
}
