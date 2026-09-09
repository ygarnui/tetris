#include "creator_logical_device.h"
#include "struct_base_data.h"

#include <logger_instance.h>

#include <stdexcept>
#include <memory>
#include <ranges>
#include <cstring>

namespace render
{

namespace
{
	bool hasExtension(const std::vector<const char*>& extensions, const char* name)
	{
		for (const char* extension : extensions)
		{
			if (std::strcmp(extension, name) == 0)
			{
				return true;
			}
		}
		return false;
	}
}
std::shared_ptr<DataDevice> CreatorLogicalDevice::CreateLogicalDevice(
	VkPhysicalDevice physicalDevice,
	const std::vector<QueueFamilyIndices>& queueFamilyIndex,
	const VkPhysicalDeviceFeatures& physicalDeviceFeatures,
	const std::vector<const char*>& deviceExtensions,
	std::shared_ptr<DataInstance> instance)
{
	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfo(queueFamilyIndex.size());
	float queuePriorities = 1.0f;
	// TO DO c++ 23
	//for (std::tuple<VkDeviceQueueCreateInfo&, uint32_t&> elem : std::views::zip(deviceQueueCreateInfo, queueFamilyIndex))
	//{
	//	initDeviceQueueCreateInfo(std::get<0>(elem), std::get<1>(elem), queuePriorities);
	//}
	
	for (size_t i = 0; i < queueFamilyIndex.size(); i++)
	{
		auto val = queueFamilyIndex[i].getIndex();
		if (val.has_value())
		{
			initDeviceQueueCreateInfo(deviceQueueCreateInfo[i], val.value(), queuePriorities);
		}
	}

	VkDeviceCreateInfo deviceCreateInfo;
	initDeviceCreateInfo(deviceCreateInfo, deviceQueueCreateInfo, physicalDeviceFeatures, deviceExtensions);

	const bool rayTracingRequested = hasExtension(deviceExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
	const bool accelerationStructureRequested = hasExtension(deviceExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

	// Enable the scalar block layout extension so that the glsl structure has the same alignment as in C++
	// and it will be possible to use struct back and forth.
	VkPhysicalDeviceVulkan12Features vulkan12Features = {};
	vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	vulkan12Features.scalarBlockLayout = VK_TRUE;

	// The acceleration structure extension builds on the 1.2 core features below:
	// buffer device addresses are how a top level structure references its instance buffer,
	// and the descriptor indexing features are required by the extension specification.
	if (accelerationStructureRequested)
	{
		vulkan12Features.bufferDeviceAddress = VK_TRUE;
		vulkan12Features.descriptorIndexing = VK_TRUE;
		vulkan12Features.runtimeDescriptorArray = VK_TRUE;
		vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	}

	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {};
	accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	accelerationStructureFeatures.accelerationStructure = VK_TRUE;

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {};
	rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;

	deviceCreateInfo.pNext = &vulkan12Features;

	if (accelerationStructureRequested)
	{
		vulkan12Features.pNext = &accelerationStructureFeatures;

		if (rayTracingRequested)
		{
			accelerationStructureFeatures.pNext = &rayTracingPipelineFeatures;
		}
	}

	std::shared_ptr<DataDevice> device(new DataDevice{ {}, instance }, [](DataDevice* p) {
		// The allocator holds pooled VkDeviceMemory blocks carved out of this device, so it
		// must be torn down first - vkDestroyDevice requires every child object gone already.
		vmaDestroyAllocator(p->allocator);
		vkDestroyDevice(p->device, nullptr);
		delete p;
	});

	auto res = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device->device);
	if (res != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorLogicalDevice::CreateLogicalDevice] failed to create logical device!");
	}

	VmaAllocatorCreateInfo allocatorCreateInfo{};
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device->device;
	allocatorCreateInfo.instance = instance->instance;
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;

	if (accelerationStructureRequested)
	{
		allocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	}

	const VkResult vmaRes = vmaCreateAllocator(&allocatorCreateInfo, &device->allocator);
	if (vmaRes != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorLogicalDevice::CreateLogicalDevice] failed to create the VMA allocator!");
	}

	return device;
}

void CreatorLogicalDevice::initDeviceQueueCreateInfo(
	VkDeviceQueueCreateInfo& queueCreateInfo, 
	const uint32_t index, 
	float& queuePriority)
{
	queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = index;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;
}

void CreatorLogicalDevice::initDeviceCreateInfo(
	VkDeviceCreateInfo& deviceCreateInfo, 
	const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfo, 
	const VkPhysicalDeviceFeatures& deviceFeatures,
	const std::vector<const char*>& deviceExtensions)
{
	deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfo.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size());

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
}

}
