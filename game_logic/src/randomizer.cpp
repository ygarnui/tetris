#include "randomizer.h"

#include <algorithm>

namespace tetris::game
{

SevenBagRandomizer::SevenBagRandomizer(uint32_t seed)
	: engine_(seed)
{
}

void SevenBagRandomizer::refillBag()
{
	bag_ = { PieceType::I, PieceType::O, PieceType::T, PieceType::S, PieceType::Z, PieceType::J, PieceType::L };
	std::shuffle(bag_.begin(), bag_.end(), engine_);
	nextIndex_ = 0;
}

PieceType SevenBagRandomizer::Next()
{
	if (nextIndex_ >= bag_.size())
	{
		refillBag();
	}

	return bag_[nextIndex_++];
}

}
