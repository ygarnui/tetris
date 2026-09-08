#include "tetromino.h"

#include <cstddef>

namespace tetris::game
{

namespace
{
	using Shape = std::array<std::array<Point, 4>, 4>; // indexed by Rotation

	// Standard guideline cell layouts, as offsets in a 4x4 box, one array per rotation.
	constexpr Shape kShapeI = { {
		{ { {0,1},{1,1},{2,1},{3,1} } },
		{ { {2,0},{2,1},{2,2},{2,3} } },
		{ { {0,2},{1,2},{2,2},{3,2} } },
		{ { {1,0},{1,1},{1,2},{1,3} } },
	} };

	constexpr Shape kShapeO = { {
		{ { {1,0},{2,0},{1,1},{2,1} } },
		{ { {1,0},{2,0},{1,1},{2,1} } },
		{ { {1,0},{2,0},{1,1},{2,1} } },
		{ { {1,0},{2,0},{1,1},{2,1} } },
	} };

	constexpr Shape kShapeT = { {
		{ { {1,0},{0,1},{1,1},{2,1} } },
		{ { {1,0},{1,1},{2,1},{1,2} } },
		{ { {0,1},{1,1},{2,1},{1,2} } },
		{ { {1,0},{0,1},{1,1},{1,2} } },
	} };

	constexpr Shape kShapeS = { {
		{ { {1,0},{2,0},{0,1},{1,1} } },
		{ { {1,0},{1,1},{2,1},{2,2} } },
		{ { {1,1},{2,1},{0,2},{1,2} } },
		{ { {0,0},{0,1},{1,1},{1,2} } },
	} };

	constexpr Shape kShapeZ = { {
		{ { {0,0},{1,0},{1,1},{2,1} } },
		{ { {2,0},{1,1},{2,1},{1,2} } },
		{ { {0,1},{1,1},{1,2},{2,2} } },
		{ { {1,0},{0,1},{1,1},{0,2} } },
	} };

	constexpr Shape kShapeJ = { {
		{ { {0,0},{0,1},{1,1},{2,1} } },
		{ { {1,0},{2,0},{1,1},{1,2} } },
		{ { {0,1},{1,1},{2,1},{2,2} } },
		{ { {1,0},{1,1},{0,2},{1,2} } },
	} };

	constexpr Shape kShapeL = { {
		{ { {2,0},{0,1},{1,1},{2,1} } },
		{ { {1,0},{1,1},{1,2},{2,2} } },
		{ { {0,1},{1,1},{2,1},{0,2} } },
		{ { {0,0},{1,0},{1,1},{1,2} } },
	} };
}

std::array<Point, 4> GetCells(PieceType type, Rotation rotation)
{
	const std::size_t rotationIndex = static_cast<std::size_t>(rotation);

	switch (type)
	{
		case PieceType::I: return kShapeI[rotationIndex];
		case PieceType::O: return kShapeO[rotationIndex];
		case PieceType::T: return kShapeT[rotationIndex];
		case PieceType::S: return kShapeS[rotationIndex];
		case PieceType::Z: return kShapeZ[rotationIndex];
		case PieceType::J: return kShapeJ[rotationIndex];
		case PieceType::L: return kShapeL[rotationIndex];
	}

	return kShapeI[0]; // unreachable, every PieceType is handled above
}

Rotation NextRotation(Rotation rotation) noexcept
{
	switch (rotation)
	{
		case Rotation::R0:   return Rotation::R90;
		case Rotation::R90:  return Rotation::R180;
		case Rotation::R180: return Rotation::R270;
		case Rotation::R270: return Rotation::R0;
	}

	return Rotation::R0; // unreachable, every Rotation is handled above
}

}
