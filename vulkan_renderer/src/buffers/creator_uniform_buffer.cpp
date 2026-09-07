#include "creator_uniform_buffer.h"
#include "creator_buffer.h"

namespace render
{
	std::shared_ptr<DataUniformBuffer> CreatorUniformBuffer::CreateUniformBuffer(
		const uint64_t size,
		const VkBufferUsageFlags usage,
		std::shared_ptr<DataDevice> device,
		VkPhysicalDevice physicalDevice)
	{
		std::shared_ptr<DataUniformBuffer> buf(
			new DataUniformBuffer(),
			[](DataUniformBuffer* p)
			{
				delete p;
			});

		buf->buf_data = CreatorBuffer::CreateBuffer(size, usage, device);
		buf->device_memory = CreatorBuffer::CreateDeviceMemoryAndBindBuffer(
			buf->buf_data,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			physicalDevice);

		return buf;
	}
}
