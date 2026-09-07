#include "creator_instance.h"
#include "creator_debug_utils_messenger.h"
#include "struct_base_data.h"

#include <logger_instance.h>

#include <iostream>

namespace render
{

std::shared_ptr<DataInstance> CreatorInstance::CreateInstace(const std::vector<const char*>& extensions, const std::vector<const char*>& layers)
{
    VkApplicationInfo application_info;
    initApplicationInfo(application_info);

    std::shared_ptr<DataInstance> instance(new DataInstance(), [](DataInstance* p) {
        vkDestroyInstance(p->instance, nullptr);
        if (p)
        { 
            delete p;
        }
    });

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo;
    CreatorDebugUtilMessenger::initDebugMessengerCreateInfo(messengerCreateInfo);
     
    std::vector<const char*> newExtensions = extensions;
    newExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    VkInstanceCreateInfo instanceCreateInfo;
    initInstanceCreateInfo(
        instanceCreateInfo,
        application_info,
        messengerCreateInfo,
        newExtensions,
        layers);                                                   

    auto res = vkCreateInstance(&instanceCreateInfo, nullptr, &instance->instance);
    if (res != VK_SUCCESS)
    {
        LOGEXC(std::runtime_error, "[CreatorInstance::CreateInstace]");
    }

    return instance;
}

void CreatorInstance::initApplicationInfo(VkApplicationInfo& applicationInfo)
{
    applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Hello Triangle";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "Engine_0_0_0";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_2;
    //applicationInfo.pNext
}

void CreatorInstance::initInstanceCreateInfo(
    VkInstanceCreateInfo& InstanceCreateInfo, 
    const VkApplicationInfo& applicationInfo, 
    VkDebugUtilsMessengerCreateInfoEXT &messengerCreateInfo,
    const std::vector<const char*>& extensions, 
    const std::vector<const char*>& layers)
{
    InstanceCreateInfo = {};
    InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    InstanceCreateInfo.pApplicationInfo = &applicationInfo;
    InstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    InstanceCreateInfo.ppEnabledLayerNames = layers.data();

    InstanceCreateInfo.pNext = &messengerCreateInfo;

    InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    InstanceCreateInfo.ppEnabledExtensionNames = extensions.data();
}

}