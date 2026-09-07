#include "creator_image_buffer.h"

#include "../buffers/creator_buffer.h"
#include "../creator_image_views.h"

#include <logger_instance.h>

namespace render
{
	std::shared_ptr<image::InterfaceImage> CreatorImageBuffer::LoadImage(
		const description::ImportImageDescription& imageInfo)
	{
		return image::ImageLoader::CreateTextureImage(imageInfo, true);
	}

	std::shared_ptr<DataBuffer> CreatorImageBuffer::CreateTextureBuffer(
		const uint64_t size,
		const VkBufferUsageFlags usage,
		std::shared_ptr<DataDevice> device)
	{
		return CreatorBuffer::CreateBuffer(size, usage, device);
	}

	std::shared_ptr<DataDeviceMemory> CreatorImageBuffer::CreateTextureDeviceMemory(
		std::shared_ptr<DataBuffer> buffer,
		VkMemoryPropertyFlags properties,
		VkPhysicalDevice physicalDevice)
	{
		return CreatorBuffer::CreateDeviceMemoryAndBindBuffer(
			buffer,
			properties,
			physicalDevice);
	}

	std::shared_ptr<DataImage> CreatorImageBuffer::CreateImage(
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		VkFormat format,
		VkImageTiling tiling, 
		VkSampleCountFlagBits sampler,
		VkImageUsageFlags usage,
		std::shared_ptr<DataDevice> device)
	{
		std::shared_ptr<DataImage> image(
			new DataImage{ VK_NULL_HANDLE, device },
			[](DataImage* p) 
			{
				vkDestroyImage(p->device->device, p->image, nullptr);
				delete p;
			});

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TO DO VK_SHARING_MODE_CONCURRENT
		imageInfo.samples = sampler;
		imageInfo.flags = 0; // Optional
		
		const VkResult result = vkCreateImage(device->device, &imageInfo, nullptr, &image->image);
		if (result != VK_SUCCESS)
		{
			LOGEXC(std::runtime_error, "[CreatorImageBuffer::CreateImage] Failed to create image!");
		}

		return image;
	}

	std::shared_ptr<DataDeviceMemory> CreatorImageBuffer::CreateImageMemory(
		std::shared_ptr<DataImage> textureImage,
		const VkMemoryPropertyFlags properties,
		VkPhysicalDevice physicalDevice)
	{
		return CreatorBuffer::CreateDeviceMemoryAndBindImageBuffer(
			textureImage,
			properties,
			physicalDevice);
	}

	void CreatorImageBuffer::TransitionImageLayout(
		VkImage image,
		VkFormat format,
		const uint32_t mipLevels,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkQueue graphicsQueue,
		std::shared_ptr<DataDevice> logicalDevice,
		std::shared_ptr<DataCommandPool> commandPool)
	{
		VkCommandBuffer commandBuffer = CreatorBuffer::BeginSingleTimeCommands(
			logicalDevice->device,
			commandPool->command_pool);

		std::vector<VkImageMemoryBarrier> barriers;
		VkImageMemoryBarrier& barrier = barriers.emplace_back();
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
			{
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else
		{
			LOGEXC(std::invalid_argument, "[CreatorImageBuffer::TransitionImageLayout] Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage,
			destinationStage,
			0,
			0, 
			nullptr,
			0, 
			nullptr,
			static_cast<uint32_t>(barriers.size()),
			barriers.data()
		);

		CreatorBuffer::EndSingleTimeCommands(
			commandBuffer, 
			graphicsQueue,
			logicalDevice->device,
			commandPool->command_pool);
	}

	void CreatorImageBuffer::InsertImageMemoryBarrier(
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
		std::shared_ptr<DataCommandPool> commandPool)
	{
		VkCommandBuffer commandBuffer = CreatorBuffer::BeginSingleTimeCommands(
			commandPool->device->device,
			commandPool->command_pool);

		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(
			commandBuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);

		CreatorBuffer::EndSingleTimeCommands(
			commandBuffer,
			graphicsQueue,
			logicalDevice->device,
			commandPool->command_pool);
	}

	void CreatorImageBuffer::CopyBufferToImage(
		VkBuffer buffer,
		VkImage image,
		const uint32_t width,
		const uint32_t height,
		VkQueue graphicsQueue,
		std::shared_ptr<DataDevice> logicalDevice,
		std::shared_ptr<DataCommandPool> commandPool)
	{
		VkCommandBuffer commandBuffer = CreatorBuffer::BeginSingleTimeCommands(
			logicalDevice->device,
			commandPool->command_pool);

		std::vector<VkBufferImageCopy> regions;
		VkBufferImageCopy& region = regions.emplace_back();
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(regions.size()),
			regions.data()
		);

		CreatorBuffer::EndSingleTimeCommands(
			commandBuffer,
			graphicsQueue,
			logicalDevice->device,
			commandPool->command_pool);
	}

	void CreatorImageBuffer::CopyImage(
		VkImage src,
		VkImage dst,
		const uint32_t width,
		const uint32_t height,
		VkQueue graphicsQueue,
		std::shared_ptr<DataDevice> logicalDevice,
		std::shared_ptr<DataCommandPool> commandPool)
	{
		VkCommandBuffer commandBuffer = CreatorBuffer::BeginSingleTimeCommands(
			logicalDevice->device,
			commandPool->command_pool);

		VkImageCopy imageCopyRegion{};
		imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.srcSubresource.layerCount = 1;
		imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.dstSubresource.layerCount = 1;
		imageCopyRegion.extent.width = width;
		imageCopyRegion.extent.height = height;
		imageCopyRegion.extent.depth = 1;

		vkCmdCopyImage(
			commandBuffer,
			src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&imageCopyRegion);

		CreatorBuffer::EndSingleTimeCommands(
			commandBuffer,
			graphicsQueue,
			logicalDevice->device,
			commandPool->command_pool);
	}

	std::shared_ptr<DataImageView> CreatorImageBuffer::CreateImageView(
		std::shared_ptr<DataDevice> device,
		std::shared_ptr<DataImage> image,
		const VkFormat format,
		const VkImageAspectFlags aspectFlags,
		const VkComponentMapping& components)
	{
		// TO DO
		auto imageViews = CreatorImageView::CreateImageViews(
			device, 
			{ image->image },
			format,
			aspectFlags,
			components);

		return imageViews[0];
	}

	void CreatorImageBuffer::GenerateMipmaps(
		VkImage image,
		VkFormat format,
		const uint32_t width,
		const uint32_t height,
		const uint32_t mipLevels,
		VkQueue graphicsQueue,
		VkPhysicalDevice physicalDevice,
		std::shared_ptr<DataDevice> logicalDevice,
		std::shared_ptr<DataCommandPool> commandPool)
	{
		VkFormatProperties formatProperties{};
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			LOGEXC(std::runtime_error, "[CreatorImageBuffer::GenerateMipmaps] MeshTexture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = CreatorBuffer::BeginSingleTimeCommands(
			logicalDevice->device,
			commandPool->command_pool);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = width;
		int32_t mipHeight = height;

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		CreatorBuffer::EndSingleTimeCommands(
			commandBuffer,
			graphicsQueue,
			logicalDevice->device,
			commandPool->command_pool);
	}
}
