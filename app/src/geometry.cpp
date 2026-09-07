#include "geometry.h"

namespace tetris
{

Mesh CreateUnitCube()
{
	Mesh mesh;
	mesh.vertices.reserve(24);
	mesh.indices.reserve(36);

	const glm::vec3 normals[6] = {
		{  0.0f,  0.0f,  1.0f },
		{  0.0f,  0.0f, -1.0f },
		{  1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f, -1.0f,  0.0f },
	};

	// Two in plane axes per face, so the four corners can be generated the same way for all of them.
	const glm::vec3 tangents[6] = {
		{  1.0f,  0.0f,  0.0f },
		{ -1.0f,  0.0f,  0.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f,  1.0f },
		{  1.0f,  0.0f,  0.0f },
		{  1.0f,  0.0f,  0.0f },
	};

	const glm::vec3 bitangents[6] = {
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  1.0f,  0.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f,  1.0f },
	};

	for (uint32_t face = 0; face < 6; ++face)
	{
		const glm::vec3 normal = normals[face];
		const glm::vec3 tangent = tangents[face];
		const glm::vec3 bitangent = bitangents[face];
		const glm::vec3 center = normal * 0.5f;

		const uint32_t firstVertex = static_cast<uint32_t>(mesh.vertices.size());

		mesh.vertices.push_back({ center - tangent * 0.5f - bitangent * 0.5f, normal });
		mesh.vertices.push_back({ center + tangent * 0.5f - bitangent * 0.5f, normal });
		mesh.vertices.push_back({ center + tangent * 0.5f + bitangent * 0.5f, normal });
		mesh.vertices.push_back({ center - tangent * 0.5f + bitangent * 0.5f, normal });

		mesh.indices.push_back(firstVertex + 0);
		mesh.indices.push_back(firstVertex + 1);
		mesh.indices.push_back(firstVertex + 2);

		mesh.indices.push_back(firstVertex + 0);
		mesh.indices.push_back(firstVertex + 2);
		mesh.indices.push_back(firstVertex + 3);
	}

	return mesh;
}

}
