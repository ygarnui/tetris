#include "game.h"

#include "tetromino.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace tetris::game
{

namespace
{
	constexpr float kSoftDropIntervalSeconds = 0.05f;
	constexpr float kLockDelaySeconds = 0.5f;
	constexpr int kGravityLevelCap = 20;

	// Tried in order until one leaves the piece in a legal spot. A fixed, direction-agnostic
	// stand-in for full SRS wall kicks, which exist to serve two rotation directions plus
	// T-spin detection - neither of which this single-rotate-button cabinet has a use for.
	constexpr std::array<Point, 4> kRotationKicks = { { {0,0}, {-1,0}, {1,0}, {0,-1} } };

	[[nodiscard]] int ScoreForClear(int rowsCleared, int level) noexcept
	{
		switch (rowsCleared)
		{
			case 1: return 100 * level;
			case 2: return 300 * level;
			case 3: return 500 * level;
			case 4: return 800 * level;
			default: return 0;
		}
	}
}

Game::Game(uint32_t seed)
	: randomizer_(seed)
	, nextType_(randomizer_.Next())
{
}

void Game::Start()
{
	board_ = Board{};
	score_ = 0;
	level_ = 1;
	lines_ = 0;
	gravityTimer_ = 0.0f;
	lockTimer_ = 0.0f;
	softDropActive_ = false;
	state_ = GameState::Running;

	spawnPiece();
}

void Game::TogglePause()
{
	if (state_ == GameState::Running) { state_ = GameState::Paused; }
	else if (state_ == GameState::Paused) { state_ = GameState::Running; }
}

void Game::spawnPiece()
{
	activeType_ = nextType_;
	nextType_ = randomizer_.Next();
	activeRotation_ = Rotation::R0;
	activePosition_ = { (Board::kWidth - 4) / 2, 0 };

	if (!board_.CanPlace(GetCells(activeType_, activeRotation_), activePosition_))
	{
		state_ = GameState::GameOver;
		return;
	}

	gravityTimer_ = 0.0f;
	lockTimer_ = 0.0f;
}

bool Game::isGrounded() const noexcept
{
	const Point below{ activePosition_.x, activePosition_.y + 1 };
	return !board_.CanPlace(GetCells(activeType_, activeRotation_), below);
}

float Game::gravityIntervalSeconds() const noexcept
{
	const int clampedLevel = std::min(level_, kGravityLevelCap);
	return std::pow(0.8f - static_cast<float>(clampedLevel - 1) * 0.007f, static_cast<float>(clampedLevel - 1));
}

void Game::Tick(float deltaSeconds)
{
	if (state_ != GameState::Running) { return; }

	if (isGrounded())
	{
		lockTimer_ += deltaSeconds;
		if (lockTimer_ >= kLockDelaySeconds)
		{
			lockActivePiece();
		}
		return;
	}

	lockTimer_ = 0.0f;
	gravityTimer_ += deltaSeconds;

	const float interval = softDropActive_
		? std::min(gravityIntervalSeconds(), kSoftDropIntervalSeconds)
		: gravityIntervalSeconds();

	if (gravityTimer_ >= interval)
	{
		gravityTimer_ = 0.0f;
		activePosition_.y += 1;
	}
}

void Game::lockActivePiece()
{
	board_.Place(GetCells(activeType_, activeRotation_), activePosition_, activeType_);

	const int cleared = board_.ClearFullRows();
	if (cleared > 0)
	{
		lines_ += cleared;
		score_ += ScoreForClear(cleared, level_);
		level_ = 1 + lines_ / 10;
	}

	spawnPiece();
}

void Game::MoveLeft()
{
	if (state_ != GameState::Running) { return; }

	const Point trial{ activePosition_.x - 1, activePosition_.y };
	if (board_.CanPlace(GetCells(activeType_, activeRotation_), trial))
	{
		activePosition_ = trial;
	}
}

void Game::MoveRight()
{
	if (state_ != GameState::Running) { return; }

	const Point trial{ activePosition_.x + 1, activePosition_.y };
	if (board_.CanPlace(GetCells(activeType_, activeRotation_), trial))
	{
		activePosition_ = trial;
	}
}

void Game::Rotate()
{
	if (state_ != GameState::Running) { return; }

	const Rotation newRotation = NextRotation(activeRotation_);
	const std::array<Point, 4> cells = GetCells(activeType_, newRotation);

	for (const Point& kick : kRotationKicks)
	{
		const Point trial = activePosition_ + kick;
		if (board_.CanPlace(cells, trial))
		{
			activeRotation_ = newRotation;
			activePosition_ = trial;
			return;
		}
	}
}

void Game::SetSoftDrop(bool active) noexcept
{
	softDropActive_ = active;
}

void Game::HardDrop()
{
	if (state_ != GameState::Running) { return; }

	while (!isGrounded())
	{
		activePosition_.y += 1;
	}

	lockActivePiece();
}

}
