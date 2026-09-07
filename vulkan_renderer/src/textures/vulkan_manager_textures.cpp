#include "vulkan_manager_textures.h"

#include "creator_image_buffer.h"
#include "../buffers/creator_buffer.h"
#include "../buffers/creator_descriptor_set.h"
#include "../buffers/creator_descriptor_pool.h"
#include "../buffers/creator_descriptor_set_layout.h"
#include "../converter_description.h"

#include <logger_instance.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <guard_next_id.h>

namespace render
{
	VulkanManagerTextures::VulkanManagerTextures()
	{

	}

	std::shared_ptr<VulkanManagerTextures>& VulkanManagerTextures::Get()
	{
		static std::shared_ptr<VulkanManagerTextures> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<VulkanManagerTextures>(new VulkanManagerTextures);
		}
		return manager;
	}

	VulkanManagerTextures::~VulkanManagerTextures()
	{
		LOG(Loglvl::debug, "[VulkanManagerTextures::~VulkanManagerTextures]");
	}

	TextureId VulkanManagerTextures::AddTexture(
		const description::ImportImageDescription& imageInfo,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId)
	{
		auto image = CreatorImageBuffer::LoadImage(imageInfo);

		return AddTexture(image, VK_SAMPLE_COUNT_1_BIT, logicalDeviceId, physicalDeviceId);
	}

	TextureId VulkanManagerTextures::AddTexture(
		std::shared_ptr<image::InterfaceImage> imageSourceData,
		VkSampleCountFlagBits sampler,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId)
	{
		const VkFormat format = ConverterDescription::FormatToVkFormat(imageSourceData->GetFormat());

		const uint32_t mipLevels = imageSourceData->IsMipmapsEnabled() * static_cast<uint32_t>(std::floor(std::log2(std::max(
			imageSourceData->GetTexWidth(), imageSourceData->GetTexHeight())))) + 1;

		VkImageUsageFlags usage = 0;
		VkImageAspectFlags aspect = 0;
		switch (imageSourceData->GetUsage())
		{
		case description::TextureUsage::TEXTURE:
		{
			usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
		}
		case description::TextureUsage::COLOR_ATTACHMENT:
		{
			usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
		}
		case description::TextureUsage::DEPTH_ATTACHMENT:
		{
			usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			break;
		}		
		case description::TextureUsage::STORAGE_TEXTURE:
		{
			usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
		}
		}

		VkImageTiling tiling{};
		if (imageSourceData->GetMemoryAccess() == description::MemoryAccess::GPU)
		{
			tiling = VK_IMAGE_TILING_OPTIMAL;
		}
		else if (imageSourceData->GetMemoryAccess() == description::MemoryAccess::CPU)
		{
			tiling = VK_IMAGE_TILING_LINEAR;
		}

		auto image = CreatorImageBuffer::CreateImage(
			imageSourceData->GetTexWidth(),
			imageSourceData->GetTexHeight(),
			mipLevels,
			format,
			tiling,
			sampler,
			usage,
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId));

		auto textureId = GeneratorId::GenerateUniqueId<TextureId>(next_texture_id_);

		auto guardResize = utils::GuardResize::MayBeResize(
			next_texture_id_,
			[](const Texture& texture) { return bool(!texture.image_data); },
			textures_);

		Texture texture{};
		texture.logical_device_id = logicalDeviceId;
		texture.size = { imageSourceData->GetTexWidth(), imageSourceData->GetTexHeight() };
		texture.image_data = image;

		VkMemoryPropertyFlags memoryProperties = 0;
		if (imageSourceData->GetMemoryAccess() == description::MemoryAccess::GPU)
		{
			memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		else if (imageSourceData->GetMemoryAccess() == description::MemoryAccess::CPU)
		{
			memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}

		auto deviceMemory = CreatorImageBuffer::CreateImageMemory(
			image,
			memoryProperties,
			ManagerDevice::Get()->GetPhysicalDevice(physicalDeviceId));

		texture.device_memory_data = deviceMemory;
		texture.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if (imageSourceData->GetPixels() && imageSourceData->GetSize() > 0)
		{
			auto stagingBuffer = CreatorImageBuffer::CreateTextureBuffer(
				imageSourceData->GetSize(),
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId));

			auto stagingBufferMemory = CreatorImageBuffer::CreateTextureDeviceMemory(
				stagingBuffer,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				ManagerDevice::Get()->GetPhysicalDevice(physicalDeviceId));

			void* mappedData = CreatorBuffer::Map(imageSourceData->GetSize(), 0, stagingBufferMemory);
			memcpy(mappedData, imageSourceData->GetPixels(), imageSourceData->GetSize());
			CreatorBuffer::Unmap(stagingBufferMemory);

			CreatorImageBuffer::TransitionImageLayout(
				image->image,
				format,
				mipLevels,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId),
				ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
				ManagerDevice::Get()->GetCommandPool(logicalDeviceId));

			CreatorImageBuffer::CopyBufferToImage(
				stagingBuffer->buffer,
				image->image,
				imageSourceData->GetTexWidth(),
				imageSourceData->GetTexHeight(),
				ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId),
				ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
				ManagerDevice::Get()->GetCommandPool(logicalDeviceId));

			if (mipLevels > 1)
			{
				CreatorImageBuffer::GenerateMipmaps(
					image->image,
					format,
					imageSourceData->GetTexWidth(),
					imageSourceData->GetTexHeight(),
					mipLevels,
					ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId),
					ManagerDevice::Get()->GetPhysicalDevice(physicalDeviceId),
					ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
					ManagerDevice::Get()->GetCommandPool(logicalDeviceId));
			}
			else
			{
				CreatorImageBuffer::TransitionImageLayout(
					image->image,
					format,
					mipLevels,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId),
					ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
					ManagerDevice::Get()->GetCommandPool(logicalDeviceId));
			}
		}

		auto imageView = CreatorImageBuffer::CreateImageView(
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			image,
			format,
			aspect,
			{});

		texture.image_view_data = imageView;

		auto samplerId = ManagerSampler::Get()->CreateSampler(logicalDeviceId, imageSourceData->GetSampler());
		texture.sampler_id = samplerId;

		textures_[textureId] = texture;

		return textureId;
	}

	TextureId VulkanManagerTextures::AddStorageTexture(
		std::shared_ptr<image::InterfaceImage> imageSourceData,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId)
	{
		const VkFormat format = ConverterDescription::FormatToVkFormat(imageSourceData->GetFormat());

		VkImageUsageFlags usage = 0;
		VkImageAspectFlags aspect = 0;
		switch (imageSourceData->GetUsage())
		{
		case description::TextureUsage::TEXTURE:
		{
			usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
		}
		case description::TextureUsage::COLOR_ATTACHMENT:
		{
			usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
		}
		case description::TextureUsage::DEPTH_ATTACHMENT:
		{
			usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			break;
		}
		case description::TextureUsage::STORAGE_TEXTURE:
		{
			usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
		}
		}

		VkImageTiling tiling{};
		if (imageSourceData->GetMemoryAccess() == description::MemoryAccess::GPU)
		{
			tiling = VK_IMAGE_TILING_OPTIMAL;
		}
		else if (imageSourceData->GetMemoryAccess() == description::MemoryAccess::CPU)
		{
			tiling = VK_IMAGE_TILING_LINEAR;
		}

		auto image = CreatorImageBuffer::CreateImage(
			imageSourceData->GetTexWidth(),
			imageSourceData->GetTexHeight(),
			1,
			format,
			tiling,
			VK_SAMPLE_COUNT_1_BIT,
			usage,
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId));

		auto textureId = GeneratorId::GenerateUniqueId<TextureId>(next_texture_id_);

		auto guardResize = utils::GuardResize::MayBeResize(
			next_texture_id_,
			[](const Texture& texture) { return bool(!texture.image_data); },
			textures_);

		Texture texture{};
		texture.logical_device_id = logicalDeviceId;
		texture.size = { imageSourceData->GetTexWidth(), imageSourceData->GetTexHeight() };
		texture.image_data = image;

		VkMemoryPropertyFlags memoryProperties = 0;
		if (imageSourceData->GetMemoryAccess() == description::MemoryAccess::GPU)
		{
			memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		else if (imageSourceData->GetMemoryAccess() == description::MemoryAccess::CPU)
		{
			memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}

		auto deviceMemory = CreatorImageBuffer::CreateImageMemory(
			image,
			memoryProperties,
			ManagerDevice::Get()->GetPhysicalDevice(physicalDeviceId));

		texture.device_memory_data = deviceMemory;

		texture.image_layout = VK_IMAGE_LAYOUT_GENERAL;
		CreatorImageBuffer::TransitionImageLayout(
			image->image,
			format,
			1,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_GENERAL,
			ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId),
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			ManagerDevice::Get()->GetCommandPool(logicalDeviceId));

		auto imageView = CreatorImageBuffer::CreateImageView(
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			image,
			format,
			aspect,
			{});

		texture.image_view_data = imageView;

		textures_[textureId] = texture;

		return textureId;
	}

	TextureId VulkanManagerTextures::CreateTexture(
		const GraphicsWindowId& windowId, 
		const description::ImportImageDescription& imageInfo)
	{
		return AddTexture(
			imageInfo,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerWindow::Get()->GetPhysicalDeviceId(windowId));
	}

	TextureId VulkanManagerTextures::CreateTexture(
		const GraphicsWindowId& windowId, 
		std::shared_ptr<image::InterfaceImage> imageSourceData)
	{
		return AddTexture(
			imageSourceData,
			VK_SAMPLE_COUNT_1_BIT,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerWindow::Get()->GetPhysicalDeviceId(windowId));
	}

	TextureId VulkanManagerTextures::CreateStorageTexture(
		const GraphicsWindowId& windowId, 
		std::shared_ptr<image::InterfaceImage> imageSourceData)
	{
		return AddStorageTexture(
			imageSourceData,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerWindow::Get()->GetPhysicalDeviceId(windowId));
	}

	void VulkanManagerTextures::DeleteTexture(const TextureId& textureId)
	{
		if (textureId.GetId() >= textures_.size())
		{
			LOGEXC(std::runtime_error, "[VulkanManagerTextures::DeleteTexture] Failed to delete texture whith id:" + textureId.GetId());
		}

		next_texture_id_ = std::min(next_texture_id_, textureId.GetId());

		Texture& texture = textures_[textureId];
		ManagerSampler::Get()->DeleteSampler(texture.logical_device_id, texture.sampler_id);
		
		textures_[textureId] = {};
	}

	void VulkanManagerTextures::Copy(
		const VkImage srcImage,
		const VkImage dstImage,
		const glm::ivec2 srcSize,
		const glm::ivec2 dstSize,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId)
	{
		if (srcSize.x != dstSize.x ||
			srcSize.y != dstSize.y)
		{
			LOGEXC(std::runtime_error, "[VulkanManagerTextures::Copy] Failed to copy images, size is not equal!");
		}

		CreatorImageBuffer::InsertImageMemoryBarrier(
			dstImage,
			0,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
			ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId),
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			ManagerDevice::Get()->GetCommandPool(logicalDeviceId));

		CreatorImageBuffer::InsertImageMemoryBarrier(
			srcImage,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
			ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId),
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			ManagerDevice::Get()->GetCommandPool(logicalDeviceId));

		CreatorImageBuffer::CopyImage(
			srcImage,
			dstImage,
			srcSize.x,
			srcSize.y,
			ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId),
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			ManagerDevice::Get()->GetCommandPool(logicalDeviceId));

		CreatorImageBuffer::InsertImageMemoryBarrier(
			srcImage,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
			ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId),
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			ManagerDevice::Get()->GetCommandPool(logicalDeviceId));

		CreatorImageBuffer::InsertImageMemoryBarrier(
			dstImage,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
			ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId),
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			ManagerDevice::Get()->GetCommandPool(logicalDeviceId));
	}

	std::shared_ptr<DataImage> VulkanManagerTextures::GetImageData(const TextureId& textureId) const
	{
		return textures_[textureId].image_data;
	}

	std::shared_ptr<DataImageView> VulkanManagerTextures::GetImageViewData(const TextureId& textureId) const
	{
		return textures_[textureId].image_view_data;
	}

	std::shared_ptr<DataSampler> VulkanManagerTextures::GetSamplerData(const TextureId& textureId) const
	{
		if(textures_[textureId].sampler_id.IsValid())
			return ManagerSampler::Get()->GetSamplerData(textures_[textureId].logical_device_id, textures_[textureId].sampler_id);
		return nullptr;
	}

	VkImageLayout VulkanManagerTextures::GetImageLayout(const TextureId& textureId) const
	{
		return textures_[textureId].image_layout;
	}

	void VulkanManagerTextures::SaveTexture(
		const std::string& filepath,
		const void* pixels,
		const size_t x,
		const size_t y,
		const size_t n)
	{
		stbi_write_png(filepath.c_str(), static_cast<int>(x), static_cast<int>(y), static_cast<int>(n), pixels, 0);
	}

	glm::ivec2 VulkanManagerTextures::GetTextureSize(const TextureId& textureId)
	{
		return textures_[textureId].size;
	}

	const uint8_t* VulkanManagerTextures::GetTextureData(const TextureId& textureId, size_t* strideBytes)
	{
		const Texture& texture = textures_[textureId];

		VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
		VkSubresourceLayout subResourceLayout;
		vkGetImageSubresourceLayout(texture.image_data->device->device, texture.image_data->image, &subResource, &subResourceLayout);

		uint8_t* data = nullptr;
		vkMapMemory(texture.image_data->device->device, texture.device_memory_data->buffer_memory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
		data += subResourceLayout.offset;

		if (strideBytes)
		{
			*strideBytes = subResourceLayout.rowPitch;
		}

		return data;
	}
}
