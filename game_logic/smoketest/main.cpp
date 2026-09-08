#include <board.h>
#include <game.h>
#include <randomizer.h>
#include <tetromino.h>

#include <array>
#include <cstdio>
#include <set>

using namespace tetris::game;

namespace
{
	int failures = 0;

	void Check(bool condition, const char* description)
	{
		if (!condition)
		{
			std::fprintf(stderr, "FAILED: %s\n", description);
			++failures;
		}
	}

	void TestTetromino()
	{
		constexpr PieceType types[] = { PieceType::I, PieceType::O, PieceType::T, PieceType::S, PieceType::Z, PieceType::J, PieceType::L };
		constexpr Rotation rotations[] = { Rotation::R0, Rotation::R90, Rotation::R180, Rotation::R270 };

		for (PieceType type : types)
		{
			for (Rotation rotation : rotations)
			{
				for (const Point& cell : GetCells(type, rotation))
				{
					Check(cell.x >= 0 && cell.x < 4 && cell.y >= 0 && cell.y < 4, "tetromino cell stays inside its 4x4 box");
				}
			}
		}

		Check(NextRotation(Rotation::R270) == Rotation::R0, "rotation wraps back to R0 after R270");
	}

	void TestBoard()
	{
		Board board;
		const std::array<Point, 4> singleCellAtOrigin = { { {0,0}, {0,0}, {0,0}, {0,0} } };

		Check(board.CanPlace(singleCellAtOrigin, { 0, 0 }), "an empty board accepts a placement");
		Check(!board.CanPlace(singleCellAtOrigin, { -1, 0 }), "the board rejects a placement off the left edge");
		Check(!board.CanPlace(singleCellAtOrigin, { Board::kWidth, 0 }), "the board rejects a placement off the right edge");

		board.Place(singleCellAtOrigin, { 0, 0 }, PieceType::T);
		Check(!board.CanPlace(singleCellAtOrigin, { 0, 0 }), "the board rejects overlapping an occupied cell");

		// Fill one row completely, one cell at a time, plus a marker cell directly above it,
		// then confirm clearing the full row slides the marker down into its place.
		constexpr int testRow = 5;
		for (int col = 0; col < Board::kWidth; ++col)
		{
			board.Place(singleCellAtOrigin, { col, testRow }, PieceType::T);
		}
		board.Place(singleCellAtOrigin, { 3, testRow - 1 }, PieceType::T);

		const int cleared = board.ClearFullRows();
		Check(cleared == 1, "exactly one full row is reported cleared");
		Check(board.At(testRow, 3).has_value(), "the marker cell slid down into the cleared row's place");
		Check(!board.At(testRow - 1, 3).has_value(), "the row above the cleared row is now empty where the marker was");
	}

	void TestRandomizer()
	{
		SevenBagRandomizer randomizer(12345);

		for (int bag = 0; bag < 20; ++bag)
		{
			std::set<PieceType> seenThisBag;
			for (int i = 0; i < 7; ++i)
			{
				seenThisBag.insert(randomizer.Next());
			}
			Check(seenThisBag.size() == 7, "every run of 7 draws contains all 7 piece types with no repeats");
		}
	}

	void TestGame()
	{
		Game game(42);
		Check(game.GetState() == GameState::NotStarted, "a fresh game has not started yet");

		game.Start();
		Check(game.GetState() == GameState::Running, "starting the game leaves it running");
		Check(game.GetScore() == 0 && game.GetLevel() == 1 && game.GetLines() == 0, "a freshly started game has zero score, level 1, zero lines");

		// Run enough ticks that several pieces are certain to have locked and respawned.
		for (int i = 0; i < 300; ++i)
		{
			game.Tick(0.1f);
		}
		Check(game.GetState() == GameState::Running, "the game is still running after many pieces lock and respawn");

		// Repeatedly hard-drop straight down without ever moving sideways: pieces stack in the
		// same columns without completing a row, so the stack eventually reaches the spawn row.
		Game stackingGame(7);
		stackingGame.Start();
		for (int i = 0; i < 500 && stackingGame.GetState() == GameState::Running; ++i)
		{
			stackingGame.HardDrop();
		}
		Check(stackingGame.GetState() == GameState::GameOver, "stacking pieces without ever clearing a line eventually ends the game");
	}
}

int main()
{
	TestTetromino();
	TestBoard();
	TestRandomizer();
	TestGame();

	if (failures == 0)
	{
		std::printf("all game_logic smoke tests passed\n");
		return 0;
	}

	std::fprintf(stderr, "%d game_logic smoke test(s) failed\n", failures);
	return 1;
}
