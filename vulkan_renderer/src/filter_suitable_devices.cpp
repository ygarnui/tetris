#include "filter_suitable_devices.h"
#include "required_device_parameters.h"
#include "creator_queue_families.h"

#include <logger_instance.h>

#include <algorithm>
#include <set>

namespace render
{

bool FilterSuitableDevices::CheckPhysicalDevice(
    const VkPhysicalDevice device, 
    std::shared_ptr<DataSurface> surface, 
    const std::vector<const char*>& deviceExtensions,
    const DataSwapchainSupportDetails& swapChainSupportDetailsData,
    PhysicalDeviceProperties& deviceProperties)
{
    bool result = true;
    result = result && isDeviceParamSuitable(device, deviceProperties.device_features);
    result = result && checkDeviceExtensionSupport(device, deviceExtensions);

    result = result && isDeviceQueueSuitable(device, surface, deviceProperties.queue_family_indices);
    if (surface)
    {
        result = result && chekSwapchainSupportDetails(swapChainSupportDetailsData);
    }

    return result;
}

bool FilterSuitableDevices::isDeviceParamSuitable(const VkPhysicalDevice& device, VkPhysicalDeviceFeatures& deviceFeatures)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    return checkDeviceParam(deviceProperties, deviceFeatures);
}

bool FilterSuitableDevices::isDeviceQueueSuitable(
    const VkPhysicalDevice device, 
    std::shared_ptr<DataSurface> surface, 
    std::vector<QueueFamilyIndices>& queueFamilyIndices)
{
    queueFamilyIndices = CreatorQueueFamilies::CreateQueueFamilyIndices(surface, device, VK_QUEUE_GRAPHICS_BIT);
    
    if (queueFamilyIndices.size() == 0)
    {
        LOG(Loglvl::info, "[FilterSuitableDevices::isDeviceQueueSuitable] the desired VkQueueFamilyProperties was not found");
        return false;
    }

    return true;
}

bool FilterSuitableDevices::checkDeviceExtensionSupport(
    const VkPhysicalDevice device, 
    const std::vector<const char*>& deviceExtensions) 
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    for (auto i : requiredExtensions)
    {
        LOG(Loglvl::info, "[FilterSuitableDevices::checkDeviceExtensionSupport] device don't have extension: ");
    }

    return requiredExtensions.empty();
}

bool FilterSuitableDevices::chekSwapchainSupportDetails(const DataSwapchainSupportDetails& swapChainSupportDetails)
{
    return !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.present_modes.empty();
}

bool FilterSuitableDevices::checkDeviceParam(
    const VkPhysicalDeviceProperties& deviceProperties,
    const VkPhysicalDeviceFeatures& deviceFeatures)
{
    bool result = true;
    result = result && CheckType(deviceFeatures.geometryShader, static_cast<VkBool32>(true), "Physical device doesn't support Geometry Shader!");
    result = result && CheckType(deviceFeatures.samplerAnisotropy, static_cast<VkBool32>(true), "Physical device doesn't support sampler anisotropy!");
    return result;
}

}
