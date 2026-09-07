#pragma once

#include <buffer_description.h>
#include "../struct_data.h"

#include <stdexcept>

namespace render
{
	class CreatorIndexBuffer
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
		[[nodiscard]] static std::shared_ptr<DataIndexBuffer> CreateIndexBuffer(
			std::shared_ptr<DataDevice> logicalDevice,
			std::shared_ptr<DataCommandPool> commandPool,
			VkPhysicalDevice physicalDevice,
			VkQueue graphicsQueue,
			const VkBufferUsageFlags usage,
			const uint64_t size,
			const description::IndexType type);

		[[nodiscard]] static std::shared_ptr<DataIndexBuffer> CreateIndexBuffer(
			std::shared_ptr<DataDevice> logicalDevice,
			std::shared_ptr<DataCommandPool> commandPool,
			VkPhysicalDevice physicalDevice,
			VkQueue graphicsQueue,
			const VkBufferUsageFlags usage,
			const uint64_t size,
			const VkIndexType type);

		static void WriteToIndexBuffer(
			std::shared_ptr<DataIndexBuffer> indexBuffer,
			const void* data,
			const uint64_t size,
			const uint64_t offset,
			std::shared_ptr<DataDevice> logicalDevice,
			std::shared_ptr<DataCommandPool> commandPool,
			VkPhysicalDevice physicalDevice,
			VkQueue graphicsQueue);

	private:

		[[nodiscard]] static VkIndexType TypeCoordsToVkFormat(const description::IndexType type);

		[[nodiscard]] static std::shared_ptr<DataIndexBuffer> createIndexBufferData(
			std::shared_ptr<DataDevice> device,
			VkPhysicalDevice physicalDevice,
			const uint64_t size,
			const VkBufferUsageFlags usage,
			const VkMemoryPropertyFlags properties,
			const VkIndexType type);
	};
}
