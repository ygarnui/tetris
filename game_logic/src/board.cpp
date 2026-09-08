#include "board.h"

#include <cstddef>

namespace tetris::game
{

bool Board::CanPlace(const std::array<Point, 4>& cells, Point origin) const noexcept
{
	for (const Point& cell : cells)
	{
		const int col = origin.x + cell.x;
		const int row = origin.y + cell.y;

		if (col < 0 || col >= kWidth || row < 0 || row >= kHeight)
		{
			return false;
		}

		if (cells_[static_cast<std::size_t>(row) * kWidth + static_cast<std::size_t>(col)].has_value())
		{
			return false;
		}
	}

	return true;
}

void Board::Place(const std::array<Point, 4>& cells, Point origin, PieceType type) noexcept
{
	for (const Point& cell : cells)
	{
		const int col = origin.x + cell.x;
		const int row = origin.y + cell.y;

		cells_[static_cast<std::size_t>(row) * kWidth + static_cast<std::size_t>(col)] = type;
	}
}

bool Board::isRowFull(int row) const noexcept
{
	for (int col = 0; col < kWidth; ++col)
	{
		if (!cells_[static_cast<std::size_t>(row) * kWidth + static_cast<std::size_t>(col)].has_value())
		{
			return false;
		}
	}

	return true;
}

int Board::ClearFullRows() noexcept
{
	int cleared = 0;
	int row = kHeight - 1;

	while (row >= 0)
	{
		if (!isRowFull(row))
		{
			--row;
			continue;
		}

		for (int destRow = row; destRow > 0; --destRow)
		{
			for (int col = 0; col < kWidth; ++col)
			{
				cells_[static_cast<std::size_t>(destRow) * kWidth + static_cast<std::size_t>(col)] =
					cells_[static_cast<std::size_t>(destRow - 1) * kWidth + static_cast<std::size_t>(col)];
			}
		}

		for (int col = 0; col < kWidth; ++col)
		{
			cells_[static_cast<std::size_t>(col)] = std::nullopt;
		}

		++cleared;
		// Whatever slid down into this row index has not been checked yet, so re-test it
		// instead of moving on.
	}

	return cleared;
}

Cell Board::At(int row, int col) const noexcept
{
	return cells_[static_cast<std::size_t>(row) * kWidth + static_cast<std::size_t>(col)];
}

}
