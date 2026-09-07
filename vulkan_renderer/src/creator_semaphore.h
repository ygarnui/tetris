#pragma once

#include "struct_data.h"

namespace render
{
class CreatorSemaphore
{
public:
	[[nodiscard]] static std::shared_ptr<DataSemaphore> CreateSemaphore(std::shared_ptr<DataDevice> device);

private:

};
}
