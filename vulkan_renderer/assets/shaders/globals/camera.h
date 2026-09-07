// Define extensions
#ifndef __cplusplus
#extension GL_EXT_scalar_block_layout : enable
#endif

#ifdef __cplusplus
	namespace shaders
	{
		using namespace glm;
#endif

struct Camera
{
	mat4 viewMat4;
	mat4 rotationMat4;
	mat4 projectionMat4;
	mat4 Transform;
	vec2 windowSize;
	vec3 position;
	vec3 lightDirection;
	float dpi;
	float time;
};

#ifdef __cplusplus
	}
#endif
