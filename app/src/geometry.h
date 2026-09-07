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

	Every object in the scene is an axis aligned box, so this one mesh backs a single bottom
	level acceleration structure that all instances reuse; their size and place come from the
	instance transform. Each face gets its own four vertices so the normals stay flat.
	\return the cube mesh
	*/
	[[nodiscard]] Mesh CreateUnitCube();
}
