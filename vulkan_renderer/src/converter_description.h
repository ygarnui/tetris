#pragma once

#include "struct_data.h"

#include <format.h>
#include <buffer_description.h>

namespace render
{
	class ConverterDescription
	{
	public:
		[[nodiscard]] static VkFormat FormatToVkFormat(const description::Format type);

		[[nodiscard]] static VkShaderStageFlagBits ShaderTypeToShaderStage(const description::ShaderType type);

		[[nodiscard]] static VkPrimitiveTopology TypeTopologyToPrimitiveTopology(const description::TypeTopology type);

		[[nodiscard]] static VkPolygonMode TypePolygonModeToVkPolygonMode(const description::TypePolygonMode type);

		[[nodiscard]] static VkAttachmentLoadOp ConvertAttachmentLoadOp(const description::AttachmentLoadOp type);

		[[nodiscard]] static VkAttachmentStoreOp ConvertAttachmentStoreOp(const description::AttachmentStoreOp type);

		[[nodiscard]] static VkSampleCountFlagBits ConvertAttachmentSamples(const description::SampleCountFlagBits flags);

		[[nodiscard]] static VkDescriptorType ConvertUniformTypeToDescriptorType(const description::UniformType type);

		[[nodiscard]] static VkVertexInputRate ConvertVertexInputRate(const description::VertexInputRate inputRate);

		[[nodiscard]] static VkImageLayout ConvertImageLayout(const description::ImageLayout layout);

		[[nodiscard]] static VkBufferUsageFlags ConvertUniformBufferType(const description::UniformBufferType type);

		[[nodiscard]] static description::UniformType ConvertUniformBufferTypeToUniformType(const description::UniformBufferType type);
	};
}
