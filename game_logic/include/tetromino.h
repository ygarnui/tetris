#pragma once

#include "types.h"

#include <array>

namespace tetris::game
{
	/*!
	\brief Cells a piece occupies at a given rotation, as offsets in an unrotated 4x4 box.
	\param[in] type which of the 7 standard pieces
	\param[in] rotation which of the 4 orientations
	\return the 4 occupied cells, as (column, row) offsets inside the box
	*/
	[[nodiscard]] std::array<Point, 4> GetCells(PieceType type, Rotation rotation);

	/*!
	\brief The orientation one rotate-button press away from `rotation`.

	Rotation only ever goes one way: the cabinet has a single rotate button, not a pair for
	clockwise and counter-clockwise.
	*/
	[[nodiscard]] Rotation NextRotation(Rotation rotation) noexcept;
}
