#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace render
{
class ValidationLayer
{
public:
	[[nodiscard]] static std::vector<VkLayerProperties> initValidationLayerSupport(const std::vector<const char*>& layers);

private:

	[[nodiscard]] static bool checkValidationLayerSupport(const std::vector<VkLayerProperties>& layers, const std::vector<const char*> validationLayers);
};
}
