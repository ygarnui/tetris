#include "creator_queue_families.h"
#include "device_property.h"
#include "struct_base_data.h"

#include <logger_instance.h>
#include <vulkan/vulkan_core.h>

#include <stdexcept>

namespace render
{
std::vector<QueueFamilyIndices> CreatorQueueFamilies::CreateQueueFamilyIndices(
	std::shared_ptr<DataSurface> surface,
	const VkPhysicalDevice physicalDevice, 
	const VkQueueFlagBits bits)
{
	std::vector<VkQueueFamilyProperties> queueFamilies = createQueueFamilyProperties(physicalDevice);

	VkBool32 presentSupport = false;
	
	std::vector<QueueFamilyIndices> queueFamilyIndices;

	const uint32_t n = static_cast<uint32_t>(queueFamilies.size());
	for (uint32_t i = 0; i < n; i++)
	{
		if (surface)
		{
			const VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface->surface, &presentSupport);
			if (result != VK_SUCCESS)
			{
				LOGEXC(std::runtime_error, "[CreatorQueueFamilies::CreateQueueFamilyIndices] Device queue doesn't support surface!");
			}
		}
		
		QueueFamilyIndices curFamily;
		if (queueFamilies[i].queueFlags & bits)
		{
			curFamily.graphicsFamily = i;
		}

		if (presentSupport)
		{
			curFamily.presentFamily = i;
		}

		if ((presentSupport && curFamily.isComplete()) || (!presentSupport && curFamily.graphicsFamily.has_value()))
		{
			queueFamilyIndices.push_back(curFamily);
		}
	}

 	return queueFamilyIndices;
}

std::vector<VkQueueFamilyProperties> CreatorQueueFamilies::createQueueFamilyProperties(const VkPhysicalDevice& physicalDevice)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	return queueFamilyProperties;
}

}
