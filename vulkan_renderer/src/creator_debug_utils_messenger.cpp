#include "creator_debug_utils_messenger.h"
#include "struct_data.h"
#include "struct_base_data.h"

#include <logger_instance.h>

namespace render
{
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

std::shared_ptr<DataDebugUtilsMessenger> CreatorDebugUtilMessenger::CreateDebugUtilsMessenger(std::shared_ptr<DataInstance> instance)
{
    std::shared_ptr<DataDebugUtilsMessenger> mesenger = std::shared_ptr<DataDebugUtilsMessenger>(new DataDebugUtilsMessenger(), [](DataDebugUtilsMessenger* p) {
        DestroyDebugUtilsMessengerEXT(p->instance->instance, p->debug_utils_messenger, nullptr);
        delete p;
    });

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
    initDebugMessengerCreateInfo(messengerCreateInfo);

	auto res = CreateDebugUtilsMessengerEXT(instance->instance, &messengerCreateInfo, nullptr, &mesenger->debug_utils_messenger);
	if (res != VK_SUCCESS)
	{
        LOGEXC(std::runtime_error, "[CreatorDebugUtilMessenger::CreateDebugUtilsMessenger]");
	}

    mesenger->instance = instance;

    return mesenger;
}

void CreatorDebugUtilMessenger::initDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional
}

VKAPI_ATTR VkBool32 VKAPI_CALL CreatorDebugUtilMessenger::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        LOG(Loglvl::info, "[CreatorDebugUtilMessenger::debugCallback] validation layer diagnostic:" , std::string(pCallbackData->pMessage));
        return VK_FALSE;
    }

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        LOG(Loglvl::info,"[CreatorDebugUtilMessenger::debugCallback] validation layer info:", std::string(pCallbackData->pMessage));
        return VK_FALSE;
    }

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        LOG(Loglvl::warning,"[CreatorDebugUtilMessenger::debugCallback] validation layer warning:", std::string(pCallbackData->pMessage));
        return VK_FALSE;
    }

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        LOG(Loglvl::error,"[CreatorDebugUtilMessenger::debugCallback] validation layer error:", std::string(pCallbackData->pMessage));
        return VK_FALSE;
    }

    return VK_TRUE;
}

}
