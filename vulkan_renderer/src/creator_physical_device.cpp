#include "creator_physical_device.h"

#include "struct_base_data.h"

#include <logger_instance.h>

#include <iostream>
#include <exception>
#include <stdexcept>

namespace render
{

std::vector<VkPhysicalDevice> CreatorPhysicalDevice::CreatePhysicalDevices(std::shared_ptr<DataInstance> instanse)
{
    uint32_t physicalDeviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instanse->instance, &physicalDeviceCount, nullptr);
    if (result != VK_SUCCESS)
    {
        LOGEXC(std::runtime_error, "[CreatorPhysicalDevice::CreatePhysicalDevices] Failed to enumerate physical devices!");
    }

    if (physicalDeviceCount == 0)
    {
        LOGEXC(std::runtime_error, "[CreatorPhysicalDevice::CreatePhysicalDevices] Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
    
    result = vkEnumeratePhysicalDevices(instanse->instance, &physicalDeviceCount, physicalDevices.data());
    if (result != VK_SUCCESS)
    {
        LOGEXC(std::runtime_error, "[CreatorPhysicalDevice::CreatePhysicalDevices] Failed to enumerate physical devices!");
    }

    return physicalDevices;
}

void CreatorPhysicalDevice::PrintInfo(const VkPhysicalDeviceProperties& physicalDeviceProperties)
{
		LOG(Loglvl::info, "apiVersion:", physicalDeviceProperties.apiVersion);
        LOG(Loglvl::info, "driverVersion:", physicalDeviceProperties.driverVersion);
        LOG(Loglvl::info, "vendorID:", physicalDeviceProperties.vendorID);
        LOG(Loglvl::info, "deviceID:", physicalDeviceProperties.deviceID);
        LOG(Loglvl::info, "deviceName:", physicalDeviceProperties.deviceName);
        uint32_t flag = 1;
        for(size_t i = 0 ; i < 7; i++)
        {
            VkSampleCountFlags curFlag = flag << i;
            if (physicalDeviceProperties.limits.framebufferColorSampleCounts & curFlag)
            {
                LOG(Loglvl::info, "support MSAA: x", std::pow(2, i));
            }
        }
}

}
