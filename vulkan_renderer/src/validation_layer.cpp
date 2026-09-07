#include "validation_layer.h"

#include <logger_instance.h>

#include <iostream>

namespace render
{

std::vector<VkLayerProperties> ValidationLayer::initValidationLayerSupport(const std::vector<const char*>& validationLayers)
{
    std::vector<VkLayerProperties> layers;
    uint32_t layerCount;
    auto res = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (res != VK_SUCCESS)
    {
        LOGEXC(std::runtime_error, "[ValidationLayer::initValidationLayerSupport]");
    }

    layers.resize(layerCount);
    res = vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
    if (res != VK_SUCCESS)
    {
        LOGEXC(std::runtime_error, "[ValidationLayer::initValidationLayerSupport]");
    }

    if (!checkValidationLayerSupport(layers, validationLayers))
    {
        LOG(Loglvl::error, "[ValidationLayer::initValidationLayerSupport]");
    }

    return layers;
}

bool ValidationLayer::checkValidationLayerSupport(const std::vector<VkLayerProperties>& layers, const std::vector<const char*> validationLayers)
{
    bool res = true;
    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : layers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            LOG(Loglvl::info, "not found layer: " , layerName);
        }
    }

    return res;
}
}
