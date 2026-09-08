#include "geometry.h"
#include <glm/geometric.hpp>

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

Mesh CreateUnitSphere()
{
	std::vector<glm::vec3> points;
	Mesh mesh;
	const uint32_t num = 8;
	const float step = 1.0f / (num - 1);
	const float halfStep = step / 2.0f;

	std::vector<uint32_t> startPos;
	startPos.push_back(static_cast<uint32_t>(points.size()));
	for(uint32_t i = 0; i < num; i++)
	{
		for(uint32_t j = 0; j < num; j++)
		{
			points.push_back({-0.5f + step * i, 0.5f, -0.5f + step * j});
		}
	}

	for(uint32_t i = 0; i < num - 1; i++)
	{
		for(uint32_t j = 0; j < num - 1; j++)
		{
			points.push_back({-0.5f + halfStep + step * i, 0.5f, -0.5f + halfStep + step * j});
		}
	}

	startPos.push_back(static_cast<uint32_t>(points.size()));
	for(uint32_t i = 0; i < num; i++)
	{
		for(uint32_t j = 0; j < num; j++)
		{
			points.push_back({-0.5f + step * i, -0.5f, -0.5f + step * j});
		}
	}

	for(uint32_t i = 0; i < num - 1; i++)
	{
		for(uint32_t j = 0; j < num - 1; j++)
		{
			points.push_back({-0.5f + halfStep + step * i, -0.5f, -0.5f + halfStep + step * j});
		}
	}

	startPos.push_back(static_cast<uint32_t>(points.size()));
	for(uint32_t i = 0; i < num; i++)
	{
		for(uint32_t j = 0; j < num; j++)
		{
			points.push_back({0.5f, -0.5f + step * i, -0.5f + step * j});
		}
	}

	for(uint32_t i = 0; i < num - 1; i++)
	{
		for(uint32_t j = 0; j < num - 1; j++)
		{
			points.push_back({0.5f, -0.5f + halfStep + step * i, -0.5f + halfStep + step * j});
		}
	}

	startPos.push_back(static_cast<uint32_t>(points.size()));
	for(uint32_t i = 0; i < num; i++)
	{
		for(uint32_t j = 0; j < num; j++)
		{
			points.push_back({-0.5f, -0.5f + step * i, -0.5f + step * j});
		}
	}

	for(uint32_t i = 0; i < num - 1; i++)
	{
		for(uint32_t j = 0; j < num - 1; j++)
		{
			points.push_back({-0.5f, -0.5f + halfStep + step * i, -0.5f + halfStep + step * j});
		}
	}

	startPos.push_back(static_cast<uint32_t>(points.size()));
	for(uint32_t i = 0; i < num; i++)
	{
		for(uint32_t j = 0; j < num; j++)
		{
			points.push_back({-0.5f + step * i, -0.5f + step * j, 0.5f});
		}
	}

	for(uint32_t i = 0; i < num - 1; i++)
	{
		for(uint32_t j = 0; j < num - 1; j++)
		{
			points.push_back({-0.5f + halfStep + step * i, -0.5f + halfStep + step * j, 0.5f});
		}
	}

	startPos.push_back(static_cast<uint32_t>(points.size()));
	for(uint32_t i = 0; i < num; i++)
	{
		for(uint32_t j = 0; j < num; j++)
		{
			points.push_back({-0.5f + step * i, -0.5f + step * j, -0.5f});
		}
	}

	for(uint32_t i = 0; i < num - 1; i++)
	{
		for(uint32_t j = 0; j < num - 1; j++)
		{
			points.push_back({-0.5f + halfStep + step * i, -0.5f + halfStep + step * j, -0.5f});
		}
	}

	for(const auto value : startPos)
	{
		for(uint32_t i = 0; i < num - 1; i++)
		{
			for(uint32_t j = 0; j < num - 1; j++)
			{
				mesh.indices.push_back(value + num * num + (num - 1) * j + i);
				mesh.indices.push_back(value + num * j + i);
				mesh.indices.push_back(value + num * j + i + 1);

				mesh.indices.push_back(value + num * num + (num - 1) * j + i);
				mesh.indices.push_back(value + num * j + i + 1);
				mesh.indices.push_back(value + num * (j + 1) + i + 1);

				mesh.indices.push_back(value + num * num + (num - 1) * j + i);
				mesh.indices.push_back(value + num * (j + 1) + i + 1);
				mesh.indices.push_back(value + num * (j + 1) + i);

				mesh.indices.push_back(value + num * num + (num - 1) * j + i);
				mesh.indices.push_back(value + num * (j + 1) + i);
				mesh.indices.push_back(value + num * j + i);
			}
		}
	}

	for(const auto& point : points)
	{
		auto value = glm::normalize(point);
		mesh.vertices.push_back({value, value});
	}

	return mesh;
}

}
