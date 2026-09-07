#include "creator_index_buffer.h"
#include "creator_buffer.h"

namespace render
{

std::shared_ptr<DataIndexBuffer> CreatorIndexBuffer::CreateIndexBuffer(
	std::shared_ptr<DataDevice> logicalDevice,
	std::shared_ptr<DataCommandPool> commandPool,
	VkPhysicalDevice physicalDevice,
	VkQueue graphicsQueue,
	const VkBufferUsageFlags usage,
	const uint64_t size,
	const description::IndexType type)
{
	return createIndexBufferData(
		logicalDevice,
		physicalDevice,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT  | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		TypeCoordsToVkFormat(type));
}

std::shared_ptr<DataIndexBuffer> CreatorIndexBuffer::CreateIndexBuffer(
	std::shared_ptr<DataDevice> logicalDevice,
	std::shared_ptr<DataCommandPool> commandPool,
	VkPhysicalDevice physicalDevice,
	VkQueue graphicsQueue,
	const VkBufferUsageFlags usage,
	const uint64_t size,
	const VkIndexType type)
{
	return createIndexBufferData(
		logicalDevice,
		physicalDevice,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		type);
}

void CreatorIndexBuffer::WriteToIndexBuffer(
	std::shared_ptr<DataIndexBuffer> indexBuffer,
	const void* data,
	const uint64_t size,
	const uint64_t offset,
	std::shared_ptr<DataDevice> logicalDevice,
	std::shared_ptr<DataCommandPool> commandPool,
	VkPhysicalDevice physicalDevice,
	VkQueue graphicsQueue)
{
	std::shared_ptr<DataIndexBuffer> srcBuf = createIndexBufferData(
		logicalDevice,
		physicalDevice,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		indexBuffer->index_type);

	void* mappedData = CreatorBuffer::Map(size, 0, srcBuf->device_memory);
	memcpy(mappedData, data, size);
	CreatorBuffer::Unmap(srcBuf->device_memory);

	CreatorBuffer::CopyBuffer(
		logicalDevice->device,
		commandPool->command_pool,
		graphicsQueue,
		srcBuf->buf_data,
		indexBuffer->buf_data,
		size,
		0,
		0);
}

VkIndexType CreatorIndexBuffer::TypeCoordsToVkFormat(const description::IndexType type)
{
	// TO DO
	return static_cast<VkIndexType>(type);
}

std::shared_ptr<DataIndexBuffer> CreatorIndexBuffer::createIndexBufferData(
	std::shared_ptr<DataDevice> device,
	VkPhysicalDevice physicalDevice,
	const uint64_t size,
	const VkBufferUsageFlags usage,
	const VkMemoryPropertyFlags properties,
	const VkIndexType type)
{
	std::shared_ptr<DataIndexBuffer> buf = std::make_shared<DataIndexBuffer>();
	buf->buf_data = CreatorBuffer::CreateBuffer(size, usage, device);

	buf->device_memory = CreatorBuffer::CreateDeviceMemoryAndBindBuffer(
		buf->buf_data,
		properties,
		physicalDevice);

	buf->index_type = type;
	uint64_t index_size = 2 + 2 * static_cast<uint64_t>(buf->index_type);
	buf->num_index = static_cast<uint32_t>(size / index_size);

	return buf;
}

}
