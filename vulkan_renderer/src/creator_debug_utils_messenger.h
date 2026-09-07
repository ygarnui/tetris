#pragma once

#include "struct_data.h"

#include <memory>

#include <vulkan/vulkan_core.h>

namespace render
{
class CreatorDebugUtilMessenger
{
public:
	[[nodiscard]] static std::shared_ptr<DataDebugUtilsMessenger> CreateDebugUtilsMessenger(std::shared_ptr<DataInstance> instance);

	static void initDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

private:

	static VKAPI_ATTR VkBool32 VKAPI_CALL  debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	//res = CreateDebugUtilsMessengerEXT(instanceData->instance, &messengerCreateInfo, nullptr, &instanceData->debug_utils_messenger);
};
}
