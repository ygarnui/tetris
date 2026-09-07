#pragma once

namespace render
{
	enum class DrawPriority
	{
		First = 0,
		Mesh,
		Line,
		Point,
		Text,
		Stroke,
		Legend,
		Compass,
		Count,
		None = Count
	};
}
