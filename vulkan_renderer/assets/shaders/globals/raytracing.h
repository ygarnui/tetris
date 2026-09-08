// Shared between the renderer and the ray tracing shaders.
// Keep every struct here layout compatible: the buffers are declared with scalar block
// layout, so a glm struct on the C++ side and the GLSL struct below match member for member.

#ifndef __cplusplus
	#extension GL_EXT_scalar_block_layout : enable
	#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
	#extension GL_EXT_buffer_reference : require
#endif

// ---------------------------------------------------------------------------------------
// Quality settings. These are the knobs to turn.
// ---------------------------------------------------------------------------------------

// How many reflection bounces follow the primary ray.
// 0 disables reflections entirely, 1 gives a single reflected image, higher values show
// reflections inside reflections. Shadows and mirror reflections are deterministic, so
// raising this does not add noise, only cost.
#define MAX_BOUNCES 1

#define AA_SAMPLES 8

// Surfaces with a reflectivity at or below this are treated as fully diffuse and stop the
// bounce loop early.
#define MIN_REFLECTIVITY 0.01

// Offset along the normal when spawning secondary rays, to keep them from hitting the
// surface they started on.
#define RAY_ORIGIN_BIAS 0.001

#define RAY_MAX_DISTANCE 10000.0

// ---------------------------------------------------------------------------------------
// Descriptor bindings of set 0. Mirrored by RayTracingBinding on the C++ side.
// ---------------------------------------------------------------------------------------

#define RT_BINDING_ACCELERATION_STRUCTURE 0
#define RT_BINDING_OUTPUT_IMAGE 1
#define RT_BINDING_CAMERA 2
#define RT_BINDING_INSTANCES 3

#ifdef __cplusplus
	#pragma once
	#include <glm/glm.hpp>
	#include <cstdint>

	namespace shaders
	{
		using namespace glm;
		using uint = uint32_t;
#endif

/*!
\brief One vertex of the scene geometry, as the hit shader reads it back.
*/
struct RtVertex
{
	vec3 position;
	vec3 normal;
};


#ifndef __cplusplus
	layout(buffer_reference, scalar) readonly buffer VertexBufferRef
	{
		RtVertex vertices[];
	};

	layout(buffer_reference, scalar) readonly buffer IndexBufferRef
	{
		uint indices[];
	};
#endif
/*!
\brief Per instance shading data, indexed by gl_InstanceCustomIndexEXT.

vertex_buffer_address and index_buffer_address are the addresses of this 
instance's own vertex/index buffer, 
so different meshes can be added without touching a shared buffer.
*/
struct RtInstance
{
	/*! \brief Surface colour in rgb, mirror reflectivity in a. */
	vec4 albedo_reflectivity;

	uint64_t vertex_buffer_address;
	uint64_t index_buffer_address;
	uint emissive;
	uint padding;
};

/*!
\brief Camera and lighting constants, rewritten every frame.
*/
struct RtCamera
{
	mat4 view_inverse;
	mat4 projection_inverse;

	/*! \brief Direction the light travels towards, xyz normalised. */
	vec4 light_direction;
	vec4 light_color;
	vec4 sky_color;
	vec4 ground_color;

	/*! \brief Number of samples with anti-aliasing enabled  */
	uint aa_samples;
};

/*!
\brief What a traced ray reports back to the ray generation shader.

All shading happens in the ray generation shader, so the hit shader only fills in what it
found. That keeps the pipeline recursion depth at one no matter how many bounces are traced.
*/
struct RtPayload
{
	vec3 position;
	vec3 normal;
	vec3 albedo;
	float reflectivity;
	/*! \brief 1 when the ray hit geometry, 0 when it escaped the scene. */
	uint hit;
};

#ifdef __cplusplus
	}
#endif
