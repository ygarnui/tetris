#include "creator_acceleration_structure.h"

#include "extension_functions.h"
#include "../buffers/creator_buffer.h"

#include <logger_instance.h>

#include <cstring>
#include <stdexcept>

namespace render
{

BufferWithMemory CreatorAccelerationStructure::CreateDeviceAddressBuffer(
	const BuildContext& context,
	const void* data,
	const uint64_t size,
	const VkBufferUsageFlags usage)
{
	if (size == 0)
	{
		LOGEXC(std::invalid_argument, "[CreatorAccelerationStructure::CreateDeviceAddressBuffer] size is 0");
	}

	BufferWithMemory result{};
	result.buffer = CreatorBuffer::CreateBuffer(
		size,
		usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		context.device);

	// Host visible memory keeps the scene setup simple: every buffer here is small and is
	// either written once or, for the instance buffer, rewritten whenever the scene moves.
	result.device_memory = CreatorBuffer::CreateDeviceMemoryAndBindBuffer(
		result.buffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		context.physical_device);

	if (data)
	{
		CreatorBuffer::Write(data, size, result.device_memory);
	}

	return result;
}

std::shared_ptr<DataAccelerationStructure> CreatorAccelerationStructure::createStructure(
	const BuildContext& context,
	const VkAccelerationStructureTypeKHR type,
	VkAccelerationStructureBuildGeometryInfoKHR& buildInfo,
	const VkAccelerationStructureBuildRangeInfoKHR& rangeInfo,
	const VkAccelerationStructureBuildSizesInfoKHR& sizeInfo)
{
	if (!ExtensionFunctions::IsLoaded())
	{
		LOGEXC(std::runtime_error, "[CreatorAccelerationStructure::createStructure] ray tracing entry points are not loaded");
	}

	// The structure lives in its own buffer, which the driver owns for the lifetime of the structure.
	BufferWithMemory storage = CreateDeviceAddressBuffer(
		context,
		nullptr,
		sizeInfo.accelerationStructureSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);

	std::shared_ptr<DataAccelerationStructure> structure(
		new DataAccelerationStructure{
			VK_NULL_HANDLE,
			0,
			storage.buffer,
			storage.device_memory,
			context.device,
		},
		[](DataAccelerationStructure* p)
		{
			if (p->acceleration_structure != VK_NULL_HANDLE)
			{
				ExtensionFunctions::DestroyAccelerationStructure(p->device->device, p->acceleration_structure, nullptr);
			}
			delete p;
		});

	VkAccelerationStructureCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	createInfo.buffer = storage.buffer->buffer;
	createInfo.size = sizeInfo.accelerationStructureSize;
	createInfo.type = type;

	VkResult res = ExtensionFunctions::CreateAccelerationStructure(
		context.device->device,
		&createInfo,
		nullptr,
		&structure->acceleration_structure);

	if (res != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorAccelerationStructure::createStructure] failed to create acceleration structure, code:", int(res));
	}

	// Scratch memory is only needed while the build command runs, so it is destroyed
	// as soon as this function returns and the build has been waited on.
	BufferWithMemory scratch = CreateDeviceAddressBuffer(
		context,
		nullptr,
		sizeInfo.buildScratchSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	buildInfo.dstAccelerationStructure = structure->acceleration_structure;
	buildInfo.scratchData.deviceAddress = CreatorBuffer::GetBufferDeviceAddress(scratch.buffer);

	const VkAccelerationStructureBuildRangeInfoKHR* pRangeInfo = &rangeInfo;

	VkCommandBuffer commandBuffer = CreatorBuffer::BeginSingleTimeCommands(
		context.device->device,
		context.command_pool);

	ExtensionFunctions::CmdBuildAccelerationStructures(commandBuffer, 1, &buildInfo, &pRangeInfo);

	// A top level build reads the bottom level structures built just before it, and the
	// ray tracing shaders read the result, so the build has to be ordered against both.
	VkMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		0,
		1, &barrier,
		0, nullptr,
		0, nullptr);

	CreatorBuffer::EndSingleTimeCommands(
		commandBuffer,
		context.queue,
		context.device->device,
		context.command_pool);

	VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
	addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	addressInfo.accelerationStructure = structure->acceleration_structure;

	structure->device_address = ExtensionFunctions::GetAccelerationStructureDeviceAddress(
		context.device->device,
		&addressInfo);

	return structure;
}

DataBottomLevel CreatorAccelerationStructure::CreateBottomLevel(
	const BuildContext& context,
	const GeometryDescription& geometry)
{
	if (!geometry.vertices || geometry.vertex_count == 0)
	{
		LOGEXC(std::invalid_argument, "[CreatorAccelerationStructure::CreateBottomLevel] geometry has no vertices");
	}
	if (!geometry.indices || geometry.index_count == 0)
	{
		LOGEXC(std::invalid_argument, "[CreatorAccelerationStructure::CreateBottomLevel] geometry has no indices");
	}
	if (geometry.index_count % 3 != 0)
	{
		LOGEXC(std::invalid_argument, "[CreatorAccelerationStructure::CreateBottomLevel] index count is not a multiple of 3");
	}

	// Storage buffer usage is what lets the closest hit shader read back normals and uvs
	// of the triangle it hit; the build itself only needs the read only input usage.
	const VkBufferUsageFlags geometryUsage =
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	DataBottomLevel result{};
	result.index_count = geometry.index_count;

	result.vertex_buffer = CreateDeviceAddressBuffer(
		context,
		geometry.vertices,
		static_cast<uint64_t>(geometry.vertex_count) * geometry.vertex_stride,
		geometryUsage);

	result.index_buffer = CreateDeviceAddressBuffer(
		context,
		geometry.indices,
		static_cast<uint64_t>(geometry.index_count) * sizeof(uint32_t),
		geometryUsage);

	VkAccelerationStructureGeometryTrianglesDataKHR triangles{};
	triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	triangles.vertexData.deviceAddress = CreatorBuffer::GetBufferDeviceAddress(result.vertex_buffer.buffer);
	triangles.vertexStride = geometry.vertex_stride;
	triangles.maxVertex = geometry.vertex_count - 1;
	triangles.indexType = VK_INDEX_TYPE_UINT32;
	triangles.indexData.deviceAddress = CreatorBuffer::GetBufferDeviceAddress(result.index_buffer.buffer);

	VkAccelerationStructureGeometryKHR geometryInfo{};
	geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	geometryInfo.geometry.triangles = triangles;
	geometryInfo.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &geometryInfo;

	const uint32_t triangleCount = geometry.index_count / 3;

	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
	sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	ExtensionFunctions::GetAccelerationStructureBuildSizes(
		context.device->device,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&buildInfo,
		&triangleCount,
		&sizeInfo);

	VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
	rangeInfo.primitiveCount = triangleCount;

	result.acceleration_structure = createStructure(
		context,
		VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		buildInfo,
		rangeInfo,
		sizeInfo);

	return result;
}

std::shared_ptr<DataAccelerationStructure> CreatorAccelerationStructure::CreateTopLevel(
	const BuildContext& context,
	const std::vector<VkAccelerationStructureInstanceKHR>& instances)
{
	if (instances.empty())
	{
		LOGEXC(std::invalid_argument, "[CreatorAccelerationStructure::CreateTopLevel] no instances");
	}

	BufferWithMemory instanceBuffer = CreateDeviceAddressBuffer(
		context,
		instances.data(),
		instances.size() * sizeof(VkAccelerationStructureInstanceKHR),
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);

	VkAccelerationStructureGeometryInstancesDataKHR instancesData{};
	instancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	instancesData.arrayOfPointers = VK_FALSE;
	instancesData.data.deviceAddress = CreatorBuffer::GetBufferDeviceAddress(instanceBuffer.buffer);

	VkAccelerationStructureGeometryKHR geometryInfo{};
	geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometryInfo.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometryInfo.geometry.instances = instancesData;
	geometryInfo.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &geometryInfo;

	const uint32_t instanceCount = static_cast<uint32_t>(instances.size());

	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
	sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	ExtensionFunctions::GetAccelerationStructureBuildSizes(
		context.device->device,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&buildInfo,
		&instanceCount,
		&sizeInfo);

	VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
	rangeInfo.primitiveCount = instanceCount;

	return createStructure(
		context,
		VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		buildInfo,
		rangeInfo,
		sizeInfo);
}

VkAccelerationStructureInstanceKHR CreatorAccelerationStructure::MakeInstance(
	const VkTransformMatrixKHR& transform,
	const DataBottomLevel& bottomLevel,
	const uint32_t instanceCustomIndex)
{
	if (!bottomLevel.acceleration_structure)
	{
		LOGEXC(std::invalid_argument, "[CreatorAccelerationStructure::MakeInstance] bottom level structure is null");
	}

	VkAccelerationStructureInstanceKHR instance{};
	instance.transform = transform;
	// The field is 24 bits wide, so the caller's index is truncated rather than silently wrapped.
	instance.instanceCustomIndex = instanceCustomIndex & 0x00FFFFFFu;
	instance.mask = 0xFF;
	instance.instanceShaderBindingTableRecordOffset = 0;
	instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	instance.accelerationStructureReference = bottomLevel.acceleration_structure->device_address;

	return instance;
}

}
