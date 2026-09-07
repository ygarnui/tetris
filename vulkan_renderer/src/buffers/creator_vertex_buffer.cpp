#include "creator_vertex_buffer.h"
#include "creator_buffer.h"
#include "../converter_description.h"

namespace render
{

std::shared_ptr<DataVertexBuffer> CreatorVertexBuffer::CreateVertexBuffer(
	std::shared_ptr<DataDevice> logicalDevice,
	std::shared_ptr<DataCommandPool> commandPool,
	VkPhysicalDevice physicalDevice,
	VkQueue graphicsQueue,
	const VkBufferUsageFlags usage,
	const uint64_t size,
	const description::MemoryAccess memoryAccess)
{
	VkMemoryPropertyFlags memoryFlags = 0x00000000;
	if (memoryAccess == description::MemoryAccess::CPU)
	{
		memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
	else
	{
		memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}

	std::shared_ptr<DataVertexBuffer> dstBuf = createVertexBufferData(
		logicalDevice,
		physicalDevice,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usage,
		memoryFlags);

	return dstBuf;
}

void CreatorVertexBuffer::WriteToVertexBuffer(
	std::shared_ptr<DataVertexBuffer> vertexBuffer,
	const void* data,
	const uint64_t size,
	const uint64_t offset,
	const description::MemoryAccess memoryAccess,
	std::shared_ptr<DataDevice> logicalDevice,
	std::shared_ptr<DataCommandPool> commandPool,
	VkPhysicalDevice physicalDevice,
	VkQueue graphicsQueue)
{
	if (memoryAccess == description::MemoryAccess::CPU)
	{
		void* mappedData = CreatorBuffer::Map(size, offset, vertexBuffer->device_memory);
		memcpy(mappedData, data, size);
		CreatorBuffer::Unmap(vertexBuffer->device_memory);
	}
	else if (memoryAccess == description::MemoryAccess::GPU)
	{
		std::shared_ptr<DataVertexBuffer> srcBuf = std::make_shared<DataVertexBuffer>();
		srcBuf->buf_data = CreatorBuffer::CreateBuffer(
			size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			logicalDevice);

		srcBuf->device_memory = CreatorBuffer::CreateDeviceMemoryAndBindBuffer(
			srcBuf->buf_data,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			physicalDevice);

		void* mappedData = CreatorBuffer::Map(size, 0, srcBuf->device_memory);
		memcpy(mappedData, data, size);
		CreatorBuffer::Unmap(srcBuf->device_memory);

		CreatorBuffer::CopyBuffer(
			logicalDevice->device,
			commandPool->command_pool,
			graphicsQueue,
			srcBuf->buf_data,
			vertexBuffer->buf_data,
			size,
			offset,
			0);
	}
}

std::shared_ptr<DataVertexBuffer> CreatorVertexBuffer::createVertexBufferData(
	std::shared_ptr<DataDevice> device,
	VkPhysicalDevice physicalDevice,
	const uint64_t size,
	const VkBufferUsageFlags usage,
	const VkMemoryPropertyFlags properties)
{
	std::shared_ptr<DataVertexBuffer> buf = std::make_shared<DataVertexBuffer>();
	buf->buf_data = CreatorBuffer::CreateBuffer(size, usage, device);

	buf->device_memory = CreatorBuffer::CreateDeviceMemoryAndBindBuffer(
		buf->buf_data,
		properties,
		physicalDevice);

	return buf;
}

}
