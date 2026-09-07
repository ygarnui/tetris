#include "creator_ray_tracing_pipeline.h"

#include "extension_functions.h"
#include "../buffers/creator_buffer.h"

#include <logger_instance.h>

#include <cstring>
#include <stdexcept>

namespace render
{

namespace
{
	/*!
	\brief Round a value up to the next multiple of alignment.
	*/
	uint32_t alignUp(const uint32_t value, const uint32_t alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}
}

VkPhysicalDeviceRayTracingPipelinePropertiesKHR CreatorRayTracingPipeline::GetPipelineProperties(
	VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR pipelineProperties{};
	pipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

	VkPhysicalDeviceProperties2 properties{};
	properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties.pNext = &pipelineProperties;

	vkGetPhysicalDeviceProperties2(physicalDevice, &properties);

	return pipelineProperties;
}

CreatorRayTracingPipeline::GroupLayout CreatorRayTracingPipeline::buildGroupLayout(
	const std::vector<VkPipelineShaderStageCreateInfo>& stages)
{
	GroupLayout layout{};

	auto makeGeneralGroup = [](const uint32_t stageIndex)
	{
		VkRayTracingShaderGroupCreateInfoKHR group{};
		group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = stageIndex;
		group.closestHitShader = VK_SHADER_UNUSED_KHR;
		group.anyHitShader = VK_SHADER_UNUSED_KHR;
		group.intersectionShader = VK_SHADER_UNUSED_KHR;
		return group;
	};

	// The table is laid out raygen, then miss, then hit, because vkCmdTraceRaysKHR addresses
	// each kind through its own region and the regions are carved out of one buffer in that order.
	for (uint32_t i = 0; i < stages.size(); ++i)
	{
		if (stages[i].stage == VK_SHADER_STAGE_RAYGEN_BIT_KHR)
		{
			layout.groups.push_back(makeGeneralGroup(i));
			layout.raygen_count++;
		}
	}

	for (uint32_t i = 0; i < stages.size(); ++i)
	{
		if (stages[i].stage == VK_SHADER_STAGE_MISS_BIT_KHR)
		{
			layout.groups.push_back(makeGeneralGroup(i));
			layout.miss_count++;
		}
	}

	for (uint32_t i = 0; i < stages.size(); ++i)
	{
		if (stages[i].stage == VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)
		{
			VkRayTracingShaderGroupCreateInfoKHR group{};
			group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			group.generalShader = VK_SHADER_UNUSED_KHR;
			group.closestHitShader = i;
			group.anyHitShader = VK_SHADER_UNUSED_KHR;
			group.intersectionShader = VK_SHADER_UNUSED_KHR;

			layout.groups.push_back(group);
			layout.hit_count++;
		}
	}

	if (layout.raygen_count != 1)
	{
		LOGEXC(std::runtime_error, "[CreatorRayTracingPipeline] a ray tracing pipeline needs exactly one raygen stage, got:", layout.raygen_count);
	}

	return layout;
}

std::shared_ptr<DataPipeline> CreatorRayTracingPipeline::CreatePipeline(
	std::shared_ptr<DataDevice> device,
	std::shared_ptr<DataPipelineLayout> pipelineLayout,
	const std::vector<VkPipelineShaderStageCreateInfo>& stages,
	const uint32_t maxRecursionDepth)
{
	if (!ExtensionFunctions::IsLoaded())
	{
		LOGEXC(std::runtime_error, "[CreatorRayTracingPipeline::CreatePipeline] ray tracing entry points are not loaded");
	}

	const GroupLayout layout = buildGroupLayout(stages);

	std::shared_ptr<DataPipeline> pipeline(
		new DataPipeline{ VK_NULL_HANDLE, device },
		[](DataPipeline* p)
		{
			vkDestroyPipeline(p->device->device, p->pipeline, nullptr);
			delete p;
		});

	VkRayTracingPipelineCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	createInfo.stageCount = static_cast<uint32_t>(stages.size());
	createInfo.pStages = stages.data();
	createInfo.groupCount = static_cast<uint32_t>(layout.groups.size());
	createInfo.pGroups = layout.groups.data();
	createInfo.maxPipelineRayRecursionDepth = maxRecursionDepth;
	createInfo.layout = pipelineLayout->pipline_layout;

	const VkResult res = ExtensionFunctions::CreateRayTracingPipelines(
		device->device,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		1,
		&createInfo,
		nullptr,
		&pipeline->pipeline);

	if (res != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorRayTracingPipeline::CreatePipeline] failed to create ray tracing pipeline, code:", int(res));
	}

	LOG(Loglvl::info, "[CreatorRayTracingPipeline::CreatePipeline] pipeline created, groups:", layout.groups.size());

	return pipeline;
}

DataShaderBindingTable CreatorRayTracingPipeline::CreateShaderBindingTable(
	const BuildContext& context,
	std::shared_ptr<DataPipeline> pipeline,
	const std::vector<VkPipelineShaderStageCreateInfo>& stages)
{
	const GroupLayout layout = buildGroupLayout(stages);
	const uint32_t groupCount = static_cast<uint32_t>(layout.groups.size());

	const VkPhysicalDeviceRayTracingPipelinePropertiesKHR properties = GetPipelineProperties(context.physical_device);

	const uint32_t handleSize = properties.shaderGroupHandleSize;
	// Every record must start at a handle alignment boundary, and every region at a base alignment one.
	const uint32_t handleStride = alignUp(handleSize, properties.shaderGroupHandleAlignment);
	const uint32_t baseAlignment = properties.shaderGroupBaseAlignment;

	std::vector<uint8_t> handles(static_cast<size_t>(handleSize) * groupCount);
	const VkResult res = ExtensionFunctions::GetRayTracingShaderGroupHandles(
		context.device->device,
		pipeline->pipeline,
		0,
		groupCount,
		handles.size(),
		handles.data());

	if (res != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorRayTracingPipeline::CreateShaderBindingTable] failed to get shader group handles, code:", int(res));
	}

	const uint32_t raygenRegionSize = alignUp(layout.raygen_count * handleStride, baseAlignment);
	const uint32_t missRegionSize = alignUp(layout.miss_count * handleStride, baseAlignment);
	const uint32_t hitRegionSize = alignUp(layout.hit_count * handleStride, baseAlignment);

	std::vector<uint8_t> table(static_cast<size_t>(raygenRegionSize) + missRegionSize + hitRegionSize, 0);

	// Copy the handles group by group into their aligned slots.
	uint32_t sourceGroup = 0;
	size_t destinationOffset = 0;

	auto copyRegion = [&](const uint32_t count, const uint32_t regionSize)
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			std::memcpy(
				table.data() + destinationOffset + static_cast<size_t>(i) * handleStride,
				handles.data() + static_cast<size_t>(sourceGroup) * handleSize,
				handleSize);
			sourceGroup++;
		}
		destinationOffset += regionSize;
	};

	copyRegion(layout.raygen_count, raygenRegionSize);
	copyRegion(layout.miss_count, missRegionSize);
	copyRegion(layout.hit_count, hitRegionSize);

	DataShaderBindingTable result{};
	result.buffer = CreatorAccelerationStructure::CreateDeviceAddressBuffer(
		context,
		table.data(),
		table.size(),
		VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR);

	const VkDeviceAddress baseAddress = CreatorBuffer::GetBufferDeviceAddress(result.buffer.buffer);

	// The raygen region is special: the spec requires its size to equal its stride.
	result.raygen_region.deviceAddress = baseAddress;
	result.raygen_region.stride = raygenRegionSize;
	result.raygen_region.size = raygenRegionSize;

	if (layout.miss_count > 0)
	{
		result.miss_region.deviceAddress = baseAddress + raygenRegionSize;
		result.miss_region.stride = handleStride;
		result.miss_region.size = missRegionSize;
	}

	if (layout.hit_count > 0)
	{
		result.hit_region.deviceAddress = baseAddress + raygenRegionSize + missRegionSize;
		result.hit_region.stride = handleStride;
		result.hit_region.size = hitRegionSize;
	}

	LOG(Loglvl::info, "[CreatorRayTracingPipeline::CreateShaderBindingTable] table built, size:", table.size());

	return result;
}

}
