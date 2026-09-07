#pragma once

#include "struct_data.h"
#include "device_property.h"

namespace render
{
class CreatorCommandPool
{
public:

	[[nodiscard]] static std::shared_ptr<DataCommandPool> CreateCommandPool(
		std::shared_ptr<DataDevice> device,
		QueueFamilyIndices queueFamilyIndices);
private:
};
}
