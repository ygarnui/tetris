#pragma once

#include "manager_sampler.h"
#include "../manager_device.h"
#include "../src/manager_window.h"
#include "../manager_base.h"

#include <generator_id.h>
#include <glm/glm.hpp>

#include <manager_textures.h>

#include <vector.h>

namespace render
{
	class VulkanManagerTextures : public ManagerTextures, public ManagerBase
	{
	public:
		static std::shared_ptr<VulkanManagerTextures>& Get();

		~VulkanManagerTextures();

		VulkanManagerTextures(const VulkanManagerTextures&) = delete;
		VulkanManagerTextures(VulkanManagerTextures&&) = delete;

		VulkanManagerTextures& operator= (const VulkanManagerTextures&) = delete;
		VulkanManagerTextures& operator= (VulkanManagerTextures&&) = delete;

		[[nodiscard]] TextureId CreateTexture(
			const GraphicsWindowId& windowId,
			const description::ImportImageDescription& imageInfo) override;

		[[nodiscard]] TextureId CreateTexture(
			const GraphicsWindowId& windowId,
			std::shared_ptr<image::InterfaceImage> imageSourceData) override;

		[[nodiscard]] TextureId CreateStorageTexture(
			const GraphicsWindowId& windowId,
			std::shared_ptr<image::InterfaceImage> imageSourceData) override;

		[[nodiscard]] TextureId AddTexture(
			std::shared_ptr<image::InterfaceImage> imageSourceData,
			VkSampleCountFlagBits sampler,
			const LogicalDeviceId& logicalDeviceId,
			const PhysicalDeviceId& physicalDeviceId);

		void DeleteTexture(const TextureId& textureId) override;

		void Copy(
			const VkImage srcImage,
			const VkImage dstImage,
			const glm::ivec2 srcSize,
			const glm::ivec2 dstSize,
			const LogicalDeviceId& logicalDeviceId,
			const PhysicalDeviceId& physicalDeviceId);

		std::shared_ptr<DataImage> GetImageData(const TextureId& textureId) const;

		std::shared_ptr<DataImageView> GetImageViewData(const TextureId& textureId) const;

		std::shared_ptr<DataSampler> GetSamplerData(const TextureId& textureId) const;

		VkImageLayout GetImageLayout(const TextureId& textureId) const;
		
		void SaveTexture(
			const std::string& filepath,
			const void* pixels,
			const size_t x,
			const size_t y,
			const size_t n) override;

		[[nodiscard]] glm::ivec2 GetTextureSize(const TextureId& textureId) override;

		/*!
		\brief Map the texture memory and returns the data.
		The texture must be created on the cpu side in order to have a mapped data pointer.
		\param textureId is the id of the texture.
		\param strideBytes describes the number of bytes between each row of texels in an image.
		If the image is linear, then strideBytes describes the layout of the image subresource in linear memory.
		For uncompressed formats (general images), strideBytes is the number of bytes
		between texels with the same x coordinate in adjacent rows (y coordinates differ by one).
		For compressed formats (swapchain images in some cases), the strideBytes is the number of bytes between
		compressed texel blocks in adjacent rows and can be used to write data to linear memory.
		\return a pointer to the texture pixel data.
		*/
		[[nodiscard]] const uint8_t* GetTextureData(const TextureId& textureId, size_t* strideBytes) override;

	private:
		VulkanManagerTextures();

		[[nodiscard]] TextureId AddTexture(
			const description::ImportImageDescription& imageInfo,
			const LogicalDeviceId& logicalDeviceId,
			const PhysicalDeviceId& physicalDeviceId) override;

		[[nodiscard]] TextureId AddStorageTexture(
			std::shared_ptr<image::InterfaceImage> imageSourceData,
			const LogicalDeviceId& logicalDeviceId,
			const PhysicalDeviceId& physicalDeviceId) override;

		size_t next_texture_id_ = 0;

		struct Texture
		{
			std::shared_ptr<DataDeviceMemory> device_memory_data = nullptr;
			std::shared_ptr<DataImage> image_data = nullptr;
			std::shared_ptr<DataImageView> image_view_data = nullptr;
			SamplerId sampler_id = GeneratorId::GenerateInvalidId<SamplerId>();
			glm::ivec2 size = { 0, 0 };
			LogicalDeviceId logical_device_id = GeneratorId::GenerateInvalidId<LogicalDeviceId>();
			VkImageLayout image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		};

		Vector<Texture, TextureId> textures_;
	};
}
