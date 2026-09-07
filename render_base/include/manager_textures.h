#pragma once

#include <generator_id.h>
#include <glm/glm.hpp>

namespace render
{
	class ManagerTextures
	{
	public:
		[[nodiscard]] virtual TextureId AddTexture(
			const description::ImportImageDescription& imageInfo,
			const LogicalDeviceId& logicalDeviceId,
			const PhysicalDeviceId& physicalDeviceId) = 0;

		[[nodiscard]] virtual TextureId AddStorageTexture(
			std::shared_ptr<image::InterfaceImage> imageSourceData,
			const LogicalDeviceId& logicalDeviceId,
			const PhysicalDeviceId& physicalDeviceId) = 0;

		[[nodiscard]] virtual TextureId CreateTexture(
			const GraphicsWindowId& windowId,
			const description::ImportImageDescription& imageInfo) = 0;

		[[nodiscard]] virtual TextureId CreateTexture(
			const GraphicsWindowId& windowId,
			std::shared_ptr<image::InterfaceImage> imageSourceData) = 0;

		[[nodiscard]] virtual TextureId CreateStorageTexture(
			const GraphicsWindowId& windowId,
			std::shared_ptr<image::InterfaceImage> imageSourceData) = 0;

		virtual void DeleteTexture(const TextureId& textureId) = 0;
		
		virtual void SaveTexture(
				const std::string& filepath,
				const void* pixels,
				const size_t x,
				const size_t y,
				const size_t n) = 0;

		[[nodiscard]] virtual glm::ivec2 GetTextureSize(const TextureId& textureId) = 0;

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
		[[nodiscard]] virtual const uint8_t* GetTextureData(const TextureId& textureId, size_t* strideBytes) = 0;
	};
}
