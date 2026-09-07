#pragma once

#include "../struct_data.h"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace render
{
	/*!
	\brief A buffer paired with the memory it is bound to.

	Acceleration structure inputs outlive the build: the closest hit shader reads the same
	vertex and index buffers the bottom level structure was built from, so they are kept
	together with the structure rather than freed after the build.
	*/
	struct BufferWithMemory
	{
		std::shared_ptr<DataBuffer> buffer;
		std::shared_ptr<DataDeviceMemory> device_memory;
	};

	/*!
	\brief Triangle geometry a bottom level acceleration structure is built from.
	*/
	struct GeometryDescription
	{
		const void* vertices = nullptr;
		uint32_t vertex_count = 0;
		/*! \brief Byte distance between two vertices; the position must be the first member. */
		uint32_t vertex_stride = 0;

		const uint32_t* indices = nullptr;
		uint32_t index_count = 0;
	};

	/*!
	\brief A bottom level acceleration structure and the geometry buffers behind it.
	*/
	struct DataBottomLevel
	{
		std::shared_ptr<DataAccelerationStructure> acceleration_structure;
		BufferWithMemory vertex_buffer;
		BufferWithMemory index_buffer;
		uint32_t index_count = 0;
	};

	/*!
	\brief Everything needed to run a build on the device.
	*/
	struct BuildContext
	{
		std::shared_ptr<DataDevice> device;
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;
		VkCommandPool command_pool = VK_NULL_HANDLE;
		VkQueue queue = VK_NULL_HANDLE;
	};

	class CreatorAccelerationStructure
	{
	public:
		/*!
		\brief Build a bottom level acceleration structure from indexed triangles.
		\param[in] context the device, command pool and queue the build is submitted on
		\param[in] geometry the vertex and index data, copied into device local buffers
		\return the built structure together with its geometry buffers
		\throw runtime_error if any of the Vulkan calls fails
		*/
		[[nodiscard]] static DataBottomLevel CreateBottomLevel(
			const BuildContext& context,
			const GeometryDescription& geometry);

		/*!
		\brief Build a top level acceleration structure over the given instances.
		\param[in] context the device, command pool and queue the build is submitted on
		\param[in] instances the instances, each already referencing a bottom level device address
		\return the built structure
		\throw runtime_error if any of the Vulkan calls fails
		*/
		[[nodiscard]] static std::shared_ptr<DataAccelerationStructure> CreateTopLevel(
			const BuildContext& context,
			const std::vector<VkAccelerationStructureInstanceKHR>& instances);

		/*!
		\brief Make an instance entry pointing at a bottom level structure.
		\param[in] transform a row major 3x4 object to world transform
		\param[in] bottomLevel the structure this instance refers to
		\param[in] instanceCustomIndex value readable in the hit shader as gl_InstanceCustomIndexEXT
		\return the filled instance, ready to be passed to CreateTopLevel
		*/
		[[nodiscard]] static VkAccelerationStructureInstanceKHR MakeInstance(
			const VkTransformMatrixKHR& transform,
			const DataBottomLevel& bottomLevel,
			const uint32_t instanceCustomIndex);

		/*!
		\brief Create a buffer, fill it from host memory and bind device local memory to it.
		\param[in] context the device the buffer is created on
		\param[in] data source bytes, may be null to leave the buffer uninitialised
		\param[in] size buffer size in bytes
		\param[in] usage buffer usage flags, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT is added
		\return the buffer and the memory bound to it
		*/
		[[nodiscard]] static BufferWithMemory CreateDeviceAddressBuffer(
			const BuildContext& context,
			const void* data,
			const uint64_t size,
			const VkBufferUsageFlags usage);

	private:
		[[nodiscard]] static std::shared_ptr<DataAccelerationStructure> createStructure(
			const BuildContext& context,
			const VkAccelerationStructureTypeKHR type,
			VkAccelerationStructureBuildGeometryInfoKHR& buildInfo,
			const VkAccelerationStructureBuildRangeInfoKHR& rangeInfo,
			const VkAccelerationStructureBuildSizesInfoKHR& sizeInfo);
	};
}
