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
	std::shared_ptr<DataDevice> device,
	const VkMemoryAllocateFlags allocateFlags)
{
	std::shared_ptr<DataDeviceMemory> deviceMemory(
		new DataDeviceMemory{
			memRequirements.size,
			{},
			device,
		},
		[](DataDeviceMemory* p)
		{
			vkFreeMemory(p->device->device, p->buffer_memory, nullptr);
			delete p;
		}
	);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;

	// Memory backing a buffer that is queried with vkGetBufferDeviceAddress has to be
	// allocated with the matching flag, otherwise the address query is invalid.
	VkMemoryAllocateFlagsInfo allocateFlagsInfo{};
	if (allocateFlags != 0)
	{
		allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		allocateFlagsInfo.flags = allocateFlags;
		allocInfo.pNext = &allocateFlagsInfo;
	}

	// TO DO optimize
	allocInfo.memoryTypeIndex = FindMemoryType(
		physicalDevice,
		memRequirements.memoryTypeBits,
		properties);

	auto res = vkAllocateMemory(device->device, &allocInfo, nullptr, &deviceMemory->buffer_memory);

	if (res != VK_SUCCESS) {
		LOGEXC(std::runtime_error, "[CreatorBuffer::CreateDeviceMemory] failed to allocate buffer memory");
	}

	return deviceMemory;
}

void CreatorBuffer::BindBufferMemory(
	std::shared_ptr<DataBuffer> buffer,
	std::shared_ptr<DataDeviceMemory> deviceMemory)
{
	vkBindBufferMemory(buffer->device->device, buffer->buffer, deviceMemory->buffer_memory, 0);
}

void CreatorBuffer::BindImageMemory(
	std::shared_ptr<DataImage> image,
	std::shared_ptr<DataDeviceMemory> deviceMemory)
{
	vkBindImageMemory(image->device->device, image->image, deviceMemory->buffer_memory, 0);
}

std::shared_ptr<DataDeviceMemory> CreatorBuffer::CreateDeviceMemoryAndBindBuffer(
	std::shared_ptr<DataBuffer> buffer,
	const VkMemoryPropertyFlags properties,
	VkPhysicalDevice physicalDevice,
	const VkMemoryAllocateFlags allocateFlags)
{
	auto requirements = CreatorBuffer::GetBufferMemoryRequirements(buffer);
	auto deviceMemory = CreatorBuffer::CreateDeviceMemory(
		requirements,
		properties,
		physicalDevice,
		buffer->device,
		allocateFlags);

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
	void* mappedData;
	const VkResult result = vkMapMemory(
		deviceMemory->device->device,
		deviceMemory->buffer_memory,
		offset,
		size,
		0,
		&mappedData);

	if (result != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorBuffer::Map] failed to map memory");
	}

	return mappedData;
}

void CreatorBuffer::Unmap(std::shared_ptr<DataDeviceMemory> deviceMemory)
{
	vkUnmapMemory(deviceMemory->device->device, deviceMemory->buffer_memory);
}

void CreatorBuffer::Write(
	const void* data,
	const uint64_t size,
	std::shared_ptr<DataDeviceMemory> deviceMemory)
{
	void* mappedData;
	const VkResult result = vkMapMemory(
		deviceMemory->device->device,
		deviceMemory->buffer_memory,
		0,
		size,
		0,
		&mappedData);

	if (result != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorBuffer::Write] failed to write to buffer");
	}

	memcpy(mappedData, data, size);

	vkUnmapMemory(deviceMemory->device->device, deviceMemory->buffer_memory);
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

uint32_t CreatorBuffer::FindMemoryType(
	VkPhysicalDevice physicalDevice,
	uint32_t typeFilter,
	VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && 
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	LOGEXC(std::runtime_error, "[CreatorBuffer::FindMemoryType] failed to find suitable memory type");
	return -1;
}

}
