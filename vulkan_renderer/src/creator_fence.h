#pragma once

#include "struct_data.h"

namespace render
{
	class CreatorFence
	{
	public:
		[[nodiscard]] static std::shared_ptr<DataFence> CreateFence(std::shared_ptr<DataDevice> device);
	private:
	};

}
