#pragma once

#include <buffer_description.h>
#include "../struct_data.h"

#include <stdexcept>

namespace render
{
	class CreatorVertexBuffer
	{
	public:
		/*!
		create new optimize NOT unpackable buffer
		\param[in] physical_device the physical device on which the memory will be allocated for the buffer
		\param[in] graphicsQueue the queue required for copying from a temporary to a permanent buffer
		\param[in] commandPoolData to create a command buffer
		\param[in] usage how the buffer will be used
		\param[in] data buffer data
		\param[in] size buffer size
		\param[in] type description of the data in the buffer
		\return smart pointer to the created buffer
		\throw runtime_error if it failed to create a buffer
		*/
		[[nodiscard]] static std::shared_ptr<DataVertexBuffer> CreateVertexBuffer(
			std::shared_ptr<DataDevice> logicalDevice,
			std::shared_ptr<DataCommandPool> commandPool,
			VkPhysicalDevice physicalDevice,
			VkQueue graphicsQueue,
			const VkBufferUsageFlags usage,
			const uint64_t size,
			const description::MemoryAccess memoryAccess);

		static void WriteToVertexBuffer(
			std::shared_ptr<DataVertexBuffer> vertexBuffer,
			const void* data,
			const uint64_t size,
			const uint64_t offset,
			const description::MemoryAccess memoryAccess,
			std::shared_ptr<DataDevice> logicalDevice,
			std::shared_ptr<DataCommandPool> commandPool,
			VkPhysicalDevice physicalDevice,
			VkQueue graphicsQueue);

	private:

		[[nodiscard]] static std::shared_ptr<DataVertexBuffer> createVertexBufferData(
			std::shared_ptr<DataDevice> device,
			VkPhysicalDevice physicalDevice,
			const uint64_t size,
			const VkBufferUsageFlags usage,
			const VkMemoryPropertyFlags properties);
	};
}
