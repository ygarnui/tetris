#pragma once

#include "types.h"

#include <array>

namespace tetris::game
{
	/*!
	\brief The playfield grid.

	Storage includes a couple of hidden rows above the visible playfield, so a freshly spawned
	piece has room to exist before it scrolls into view, matching the usual guideline behaviour.
	*/
	class Board
	{
	public:
		static constexpr int kWidth = 10;
		static constexpr int kVisibleHeight = 20;
		static constexpr int kHiddenRows = 2;
		static constexpr int kHeight = kVisibleHeight + kHiddenRows;

		/*!
		\brief Whether every one of `cells` (offsets from `origin`) is in bounds and empty.
		*/
		[[nodiscard]] bool CanPlace(const std::array<Point, 4>& cells, Point origin) const noexcept;

		/*!
		\brief Write `type` into every one of `cells` (offsets from `origin`).
		\pre CanPlace(cells, origin) is true
		*/
		void Place(const std::array<Point, 4>& cells, Point origin, PieceType type) noexcept;

		/*!
		\brief Remove every full row, compacting the rows above each one down by one.
		\return how many rows were cleared
		*/
		int ClearFullRows() noexcept;

		[[nodiscard]] Cell At(int row, int col) const noexcept;

	private:
		[[nodiscard]] bool isRowFull(int row) const noexcept;

		std::array<Cell, kWidth * kHeight> cells_{};
	};
}
