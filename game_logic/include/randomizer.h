#pragma once

#include "types.h"

#include <array>
#include <cstdint>
#include <cstddef>
#include <random>

namespace tetris::game
{
	/*!
	\brief Draws all 7 piece types once, in a random order, before repeating.

	Plain random draws can go a long time without handing out a particular piece (an I piece,
	say), which reads as unfair even in a casual game; a shuffled bag of all 7 rules that out.
	*/
	class SevenBagRandomizer
	{
	public:
		explicit SevenBagRandomizer(uint32_t seed);

		[[nodiscard]] PieceType Next();

	private:
		void refillBag();

		std::mt19937 engine_;
		std::array<PieceType, 7> bag_{};
		std::size_t nextIndex_ = 7; // forces a refill on the first call
	};
}
