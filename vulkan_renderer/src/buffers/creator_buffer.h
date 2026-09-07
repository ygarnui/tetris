#pragma once

#include "../struct_data.h"

#include <stdexcept>

namespace render
{
	class CreatorBuffer
	{
	public:

		/*!
		\brief create new unpackable buffer
		\param[in] size buffer size
		\param[in] usage description of the purpose of using the buffer, use as default value: VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
		\param[in] device the device for which this buffer is being created
		\return smart pointer to the created buffer
		\throw runtime_error if it failed to create a buffer
		*/
		[[nodiscard]] static std::shared_ptr<DataBuffer> CreateBuffer(
			const uint64_t size,
			const VkBufferUsageFlags usage,
			std::shared_ptr<DataDevice> device);

		[[nodiscard]] static VkMemoryRequirements GetBufferMemoryRequirements(std::shared_ptr<DataBuffer> buffer);

		[[nodiscard]] static VkMemoryRequirements GetImageMemoryRequirements(std::shared_ptr<DataImage> image);
		/*!
		\brief create new DeviceMemoryData
		\param[in] properties memory allocation settings, use default: VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		\param[in] physicalDevice the device on which the memory will be allocated
		\param[in] buffer the buffer for which memory will be allocated
		\param[in] allocateFlags extra allocation flags, pass VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
		for memory of a buffer whose device address will be queried
		\return smart pointer to the created DeviceMemoryData
		\throw runtime_error if it failed to create a DeviceMemoryData
		*/
		[[nodiscard]] static std::shared_ptr<DataDeviceMemory> CreateDeviceMemory(
			const VkMemoryRequirements& memRequirements,
			const VkMemoryPropertyFlags properties,
			VkPhysicalDevice physicalDevice,
			std::shared_ptr<DataDevice> device,
			const VkMemoryAllocateFlags allocateFlags = 0);

		static void BindBufferMemory(
			std::shared_ptr<DataBuffer> buffer,
			std::shared_ptr<DataDeviceMemory> deviceMemory);

		static void BindImageMemory(
			std::shared_ptr<DataImage> image,
			std::shared_ptr<DataDeviceMemory> deviceMemory);

		[[nodiscard]] static std::shared_ptr<DataDeviceMemory> CreateDeviceMemoryAndBindBuffer(
			std::shared_ptr<DataBuffer> buffer,
			const VkMemoryPropertyFlags properties,
			VkPhysicalDevice physicalDevice,
			const VkMemoryAllocateFlags allocateFlags = 0);

		/*!
		\brief Get the device address of a buffer.

		The buffer must be created with VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT and its memory
		allocated with VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT.
		\param[in] buffer the buffer to query
		\return the device address of the buffer
		*/
		[[nodiscard]] static VkDeviceAddress GetBufferDeviceAddress(std::shared_ptr<DataBuffer> buffer);

		[[nodiscard]] static std::shared_ptr<DataDeviceMemory> CreateDeviceMemoryAndBindImageBuffer(
			std::shared_ptr<DataImage> image,
			const VkMemoryPropertyFlags properties,
			VkPhysicalDevice physicalDevice);

		static void* Map(
			const uint64_t size,
			const uint64_t offset,
			std::shared_ptr<DataDeviceMemory> deviceMemory);

		static void Unmap(std::shared_ptr<DataDeviceMemory> deviceMemory);

		/*!
		\brief create new DeviceMemoryData
		\param[in] data buffer data
		\param[in] size buffer size
		\param[in] deviceMemory the device to which the data needs to be uploaded
		\return smart pointer to the created DeviceMemoryData
		\throw runtime_error if it failed to mapping device memory
		*/
		static void Write(
			const void* data,
			const uint64_t size,
			std::shared_ptr<DataDeviceMemory> deviceMemory);

		[[nodiscard]] static VkCommandBuffer BeginSingleTimeCommands(
			VkDevice logicalDevice,
			VkCommandPool commandPool);

		static void CopyBuffer(
			VkDevice logicalDevice,
			VkCommandPool commandPool,
			VkQueue graphicsQueue,
			std::shared_ptr<DataBuffer> srcBuffer,
			std::shared_ptr<DataBuffer> dstBuffer,
			const uint64_t size,
			const uint64_t dstOffset,
			const uint64_t srcOffset);

		static void EndSingleTimeCommands(
			VkCommandBuffer commandBuffer,
			VkQueue graphicsQueue,
			VkDevice logicalDevice,
			VkCommandPool commandPool);
		
		[[nodiscard]] static uint32_t FindMemoryType(
			VkPhysicalDevice physicalDevice,
			uint32_t typeFilter,
			VkMemoryPropertyFlags properties);
	};
}
