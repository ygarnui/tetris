#pragma once

#include <raytracing.h>

#include <cstdint>
#include <vector>

namespace tetris
{
	/*!
	\brief Vertex and index data of a mesh, in the layout the hit shader reads back.
	*/
	struct Mesh
	{
		std::vector<shaders::RtVertex> vertices;
		std::vector<uint32_t> indices;
	};

	/*!
	\brief Build a unit cube centred on the origin, spanning -0.5 to 0.5 on every axis.
	\return the cube mesh
	*/
	[[nodiscard]] Mesh CreateUnitCube();

	/*!
	\brief Construction of a sphere with the center at the origin and a radius of 0.5.
	\return the sphere mesh
	*/
	[[nodiscard]] Mesh CreateUnitSphere();
}
