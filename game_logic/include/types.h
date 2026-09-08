#pragma once

#include <optional>

namespace tetris::game
{
	enum class PieceType { I, O, T, S, Z, J, L };
	enum class Rotation { R0, R90, R180, R270 };
	enum class GameState { NotStarted, Running, Paused, GameOver };

	/*!
	\brief A grid coordinate: x is the column, y is the row (0 at the top).
	*/
	struct Point
	{
		int x = 0;
		int y = 0;
	};

	[[nodiscard]] inline Point operator+(const Point& lhs, const Point& rhs) noexcept
	{
		return { lhs.x + rhs.x, lhs.y + rhs.y };
	}

	/*!
	\brief A board cell: empty, or occupied by whichever piece locked into it.

	Kept as the piece type rather than a color so this module stays free of any notion of how
	a cell is drawn; a renderer maps `PieceType` to a color on its own.
	*/
	using Cell = std::optional<PieceType>;
}
