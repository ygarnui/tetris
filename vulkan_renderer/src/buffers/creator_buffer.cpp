#include "creator_buffer.h"

#include <logger_instance.h>

namespace render
{

std::shared_ptr<DataBuffer> CreatorBuffer::CreateBuffer(
	const uint64_t size,
	const VkBufferUsageFlags usage,
	std::shared_ptr<DataDevice> device)
{
	std::shared_ptr<DataBuffer> buffer(
		new DataBuffer{
			nullptr,
			size,
			{},
			device,
		},
		[](DataBuffer* p)
		{
			vkDestroyBuffer(p->device->device, p->buffer, nullptr);
			delete p;
		}
	);

	VkBufferCreateInfo bufferCreateInfo{};

	bufferCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;

	auto res = vkCreateBuffer(device->device, &bufferCreateInfo, nullptr, &buffer->buffer);

	if (res != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorBuffer::CreateBuffer] failed to create buffer");
	}

	return buffer;
}

VkMemoryRequirements CreatorBuffer::GetBufferMemoryRequirements(std::shared_ptr<DataBuffer> buffer)
{
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(buffer->device->device, buffer->buffer, &memRequirements);

	return memRequirements;
}

VkMemoryRequirements CreatorBuffer::GetImageMemoryRequirements(std::shared_ptr<DataImage> image)
{
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(image->device->device, image->image, &memRequirements);

	return memRequirements;
}

std::shared_ptr<DataDeviceMemory> CreatorBuffer::CreateDeviceMemory(
	const VkMemoryRequirements& memRequirements,
	const VkMemoryPropertyFlags properties,
	VkPhysicalDevice physicalDevice,
	std::shared_ptr<DataDevice> device)
{
	std::shared_ptr<DataDeviceMemory> deviceMemory(
		new DataDeviceMemory{
			memRequirements.size,
			{},
			device,
		},
		[](DataDeviceMemory* p)
		{
			vmaFreeMemory(p->device->allocator, p->allocation);
			delete p;
		}
	);

	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.requiredFlags = properties;

	const VkResult res = vmaAllocateMemory(
		device->allocator,
		&memRequirements,
		&allocCreateInfo,
		&deviceMemory->allocation,
		nullptr);

	if (res != VK_SUCCESS) {
		LOGEXC(std::runtime_error, "[CreatorBuffer::CreateDeviceMemory] failed to allocate buffer memory");
	}

	return deviceMemory;
}

void CreatorBuffer::BindBufferMemory(
	std::shared_ptr<DataBuffer> buffer,
	std::shared_ptr<DataDeviceMemory> deviceMemory)
{
	vmaBindBufferMemory(buffer->device->allocator, deviceMemory->allocation, buffer->buffer);
}

void CreatorBuffer::BindImageMemory(
	std::shared_ptr<DataImage> image,
	std::shared_ptr<DataDeviceMemory> deviceMemory)
{
	vmaBindImageMemory(image->device->allocator, deviceMemory->allocation, image->image);
}

std::shared_ptr<DataDeviceMemory> CreatorBuffer::CreateDeviceMemoryAndBindBuffer(
	std::shared_ptr<DataBuffer> buffer,
	const VkMemoryPropertyFlags properties,
	VkPhysicalDevice physicalDevice)
{
	auto requirements = CreatorBuffer::GetBufferMemoryRequirements(buffer);
	auto deviceMemory = CreatorBuffer::CreateDeviceMemory(
		requirements,
		properties,
		physicalDevice,
		buffer->device);

	CreatorBuffer::BindBufferMemory(buffer, deviceMemory);

	return deviceMemory;
}

VkDeviceAddress CreatorBuffer::GetBufferDeviceAddress(std::shared_ptr<DataBuffer> buffer)
{
	VkBufferDeviceAddressInfo addressInfo{};
	addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	addressInfo.buffer = buffer->buffer;

	return vkGetBufferDeviceAddress(buffer->device->device, &addressInfo);
}

std::shared_ptr<DataDeviceMemory> CreatorBuffer::CreateDeviceMemoryAndBindImageBuffer(
	std::shared_ptr<DataImage> image,
	const VkMemoryPropertyFlags properties,
	VkPhysicalDevice physicalDevice)
{
	auto requirements = CreatorBuffer::GetImageMemoryRequirements(image);
	auto deviceMemory = CreatorBuffer::CreateDeviceMemory(
		requirements,
		properties,
		physicalDevice,
		image->device);

	CreatorBuffer::BindImageMemory(image, deviceMemory);

	return deviceMemory;
}

void* CreatorBuffer::Map(
	const uint64_t size,
	const uint64_t offset,
	std::shared_ptr<DataDeviceMemory> deviceMemory)
{
	void* mappedData = nullptr;
	const VkResult result = vmaMapMemory(deviceMemory->device->allocator, deviceMemory->allocation, &mappedData);

	if (result != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorBuffer::Map] failed to map memory");
	}

	return static_cast<uint8_t*>(mappedData) + offset;
}

void CreatorBuffer::Unmap(std::shared_ptr<DataDeviceMemory> deviceMemory)
{
	vmaUnmapMemory(deviceMemory->device->allocator, deviceMemory->allocation);
}

void CreatorBuffer::Write(
	const void* data,
	const uint64_t size,
	std::shared_ptr<DataDeviceMemory> deviceMemory)
{
	void* mappedData = nullptr;
	const VkResult result = vmaMapMemory(deviceMemory->device->allocator, deviceMemory->allocation, &mappedData);

	if (result != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorBuffer::Write] failed to write to buffer");
	}

	memcpy(mappedData, data, size);

	vmaUnmapMemory(deviceMemory->device->allocator, deviceMemory->allocation);
}

VkCommandBuffer CreatorBuffer::BeginSingleTimeCommands(
	VkDevice logicalDevice,
	VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void CreatorBuffer::CopyBuffer(
	VkDevice logicalDevice,
	VkCommandPool commandPool,
	VkQueue graphicsQueue,
	std::shared_ptr<DataBuffer> srcBuffer,
	std::shared_ptr<DataBuffer> dstBuffer,
	const uint64_t size,
	const uint64_t dstOffset,
	const uint64_t srcOffset)
{
	auto commandBuffer = BeginSingleTimeCommands(logicalDevice, commandPool);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer->buffer, dstBuffer->buffer, 1, &copyRegion);

	EndSingleTimeCommands(
		commandBuffer,
		graphicsQueue,
		logicalDevice,
		commandPool);
}

void CreatorBuffer::EndSingleTimeCommands(
	VkCommandBuffer commandBuffer,
	VkQueue graphicsQueue,
	VkDevice logicalDevice,
	VkCommandPool commandPool)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

}
