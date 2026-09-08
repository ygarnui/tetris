#pragma once

#include "board.h"
#include "randomizer.h"
#include "types.h"

#include <cstdint>
#include <random>

namespace tetris::game
{
	/*!
	\brief Rules and state of one game of Tetris, with no rendering or input-device dependency.

	The caller drives it with discrete commands (`MoveLeft`, `Rotate`, ...) and a per-frame
	`Tick`; how those commands get triggered - keyboard, a raycast onto a physical cabinet
	button - is entirely up to the caller.
	*/
	class Game
	{
	public:
		explicit Game(uint32_t seed = std::random_device{}());

		void Start();
		void TogglePause();

		/*!
		\brief Advance gravity and the lock timer by one frame.
		\param[in] deltaSeconds time since the previous call
		*/
		void Tick(float deltaSeconds);

		void MoveLeft();
		void MoveRight();
		void Rotate();

		/*!
		\brief Matches a physically held button: true while held, false once released.
		*/
		void SetSoftDrop(bool active) noexcept;

		/*!
		\brief Drop the active piece to the floor and lock it immediately.

		No button on the physical cabinet does this; kept for keyboard-debug play and tests.
		*/
		void HardDrop();

		[[nodiscard]] GameState GetState() const noexcept { return state_; }
		[[nodiscard]] int GetScore() const noexcept { return score_; }
		[[nodiscard]] int GetLevel() const noexcept { return level_; }
		[[nodiscard]] int GetLines() const noexcept { return lines_; }
		[[nodiscard]] PieceType GetNextPiece() const noexcept { return nextType_; }
		[[nodiscard]] const Board& GetBoard() const noexcept { return board_; }
		[[nodiscard]] PieceType GetActivePieceType() const noexcept { return activeType_; }
		[[nodiscard]] Rotation GetActivePieceRotation() const noexcept { return activeRotation_; }
		[[nodiscard]] Point GetActivePiecePosition() const noexcept { return activePosition_; }

	private:
		void spawnPiece();
		void lockActivePiece();
		[[nodiscard]] bool isGrounded() const noexcept;
		[[nodiscard]] float gravityIntervalSeconds() const noexcept;

		Board board_;
		SevenBagRandomizer randomizer_;

		GameState state_ = GameState::NotStarted;
		int score_ = 0;
		int level_ = 1;
		int lines_ = 0;

		PieceType activeType_ = PieceType::I;
		Rotation activeRotation_ = Rotation::R0;
		Point activePosition_{};
		PieceType nextType_;

		float gravityTimer_ = 0.0f;
		float lockTimer_ = 0.0f;
		bool softDropActive_ = false;
	};
}
