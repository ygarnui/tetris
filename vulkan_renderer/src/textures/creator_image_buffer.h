#pragma once

#include "../struct_data.h"

#include <image_loader.h>

namespace render
{
	class CreatorImageBuffer
	{
	public:

		[[nodiscard]] static std::shared_ptr<image::InterfaceImage> LoadImage(
			const description::ImportImageDescription& imageInfo);

		[[nodiscard]] static std::shared_ptr<DataBuffer> CreateTextureBuffer(
			const uint64_t size,
			const VkBufferUsageFlags usage,
			std::shared_ptr<DataDevice> device);

		[[nodiscard]] static std::shared_ptr<DataDeviceMemory> CreateTextureDeviceMemory(
			std::shared_ptr<DataBuffer> buffer,
			VkMemoryPropertyFlags properties,
			VkPhysicalDevice physicalDevice);

		[[nodiscard]] static std::shared_ptr<DataImage> CreateImage(
			uint32_t width,
			uint32_t height,
			uint32_t mipLevels,
			VkFormat format,
			VkImageTiling tiling,
			VkSampleCountFlagBits sampler,
			VkImageUsageFlags usage,
			std::shared_ptr<DataDevice> device);

		[[nodiscard]] static std::shared_ptr<DataDeviceMemory> CreateImageMemory(
			std::shared_ptr<DataImage> textureImage,
			const VkMemoryPropertyFlags properties,
			VkPhysicalDevice physicalDevice);

		static void TransitionImageLayout(
			VkImage image,
			VkFormat format,
			const uint32_t mipLevels,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			VkQueue graphicsQueue,
			std::shared_ptr<DataDevice> logicalDevice,
			std::shared_ptr<DataCommandPool> commandPool);

		static void InsertImageMemoryBarrier(
			VkImage image,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange,
			VkQueue graphicsQueue,
			std::shared_ptr<DataDevice> logicalDevice,
			std::shared_ptr<DataCommandPool> commandPool);

		static void CopyBufferToImage(
			VkBuffer buffer,
			VkImage image,
			const uint32_t width,
			const uint32_t height,
			VkQueue graphicsQueue,
			std::shared_ptr<DataDevice> logicalDevice,
			std::shared_ptr<DataCommandPool> commandPool);

		static void CopyImage(
			VkImage src,
			VkImage dst,
			const uint32_t width,
			const uint32_t height,
			VkQueue graphicsQueue,
			std::shared_ptr<DataDevice> logicalDevice,
			std::shared_ptr<DataCommandPool> commandPool);

		[[nodiscard]] static std::shared_ptr<DataImageView> CreateImageView(
			std::shared_ptr<DataDevice> device,
			std::shared_ptr<DataImage> images,
			const VkFormat format,
			const VkImageAspectFlags aspectFlags,
			const VkComponentMapping& components);

		static void GenerateMipmaps(
			VkImage image,
			VkFormat format,
			const uint32_t width,
			const uint32_t height,
			const uint32_t mipLevels,
			VkQueue graphicsQueue,
			VkPhysicalDevice physicalDevice,
			std::shared_ptr<DataDevice> logicalDevice,
			std::shared_ptr<DataCommandPool> commandPool);
	};
}
