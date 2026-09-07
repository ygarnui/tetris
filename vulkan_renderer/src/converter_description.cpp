#include "converter_description.h"

#include <logger_instance.h>

namespace render
{

VkFormat ConverterDescription::FormatToVkFormat(const description::Format type)
{
	return static_cast<VkFormat>(type);
}

VkShaderStageFlagBits ConverterDescription::ShaderTypeToShaderStage(const description::ShaderType type)
{
	switch (type)
	{
	case description::ShaderType::VERTEX:
		return VK_SHADER_STAGE_VERTEX_BIT;

	case description::ShaderType::GEOMETRY:
		return VK_SHADER_STAGE_GEOMETRY_BIT;

	case description::ShaderType::COMPUTE:
		return VK_SHADER_STAGE_COMPUTE_BIT;

	case description::ShaderType::TESSELLATION_CONTROL:
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

	case description::ShaderType::TESSELLATION_EVALUATION:
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

	case description::ShaderType::FRAGMENT:
		return VK_SHADER_STAGE_FRAGMENT_BIT;

	case description::ShaderType::RAY_GENERATION:
		return VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	case description::ShaderType::RAY_MISS:
		return VK_SHADER_STAGE_MISS_BIT_KHR;

	case description::ShaderType::RAY_CLOSEST_HIT:
		return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	case description::ShaderType::RAY_ANY_HIT:
		return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

	case description::ShaderType::RAY_INTERSECTION:
		return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;

	case description::ShaderType::NONE:
		return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

	default:
		return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}
}

VkPrimitiveTopology ConverterDescription::TypeTopologyToPrimitiveTopology(const description::TypeTopology type)
{
	switch (type)
	{
	case description::TypeTopology::POINTS:
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

	case description::TypeTopology::LINE_LIST:
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

	case description::TypeTopology::LINE_STRIP:
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

	case description::TypeTopology::TRIANGLE_LIST:
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	case description::TypeTopology::TRIANGLE_STRIP:
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

	case description::TypeTopology::PRIMITIVE_TOPOLOGY_PATCH_LIST:
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

	default:
		return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	}
}

VkPolygonMode ConverterDescription::TypePolygonModeToVkPolygonMode(const description::TypePolygonMode type)
{
	switch (type)
	{
	case description::TypePolygonMode::POLYGON_MODE_FILL:
		return VK_POLYGON_MODE_FILL;

	case description::TypePolygonMode::POLYGON_MODE_LINE:
		return VK_POLYGON_MODE_LINE;

	case description::TypePolygonMode::POLYGON_MODE_POINT:
		return VK_POLYGON_MODE_POINT;

	case description::TypePolygonMode::POLYGON_MODE_FILL_RECTANGLE_NV:
		return VK_POLYGON_MODE_FILL_RECTANGLE_NV;

	default:
		return VK_POLYGON_MODE_MAX_ENUM;
	}
}

VkAttachmentLoadOp ConverterDescription::ConvertAttachmentLoadOp(const description::AttachmentLoadOp type)
{
	switch (type)
	{
	case description::AttachmentLoadOp::ATTACHMENT_LOAD_OP_LOAD:
		return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;

	case description::AttachmentLoadOp::ATTACHMENT_LOAD_OP_CLEAR:
		return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;

	case description::AttachmentLoadOp::ATTACHMENT_LOAD_OP_DONT_CARE:
		return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;

	case description::AttachmentLoadOp::NONE:
		return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_MAX_ENUM;

	default:
		return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
	}
}

VkAttachmentStoreOp ConverterDescription::ConvertAttachmentStoreOp(const description::AttachmentStoreOp type)
{
	switch (type)
	{
	case description::AttachmentStoreOp::ATTACHMENT_STORE_OP_STORE:
		return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;

	case description::AttachmentStoreOp::ATTACHMENT_STORE_OP_DONT_CARE:
		return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;

	default:
		return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_MAX_ENUM;
	}
}

VkSampleCountFlagBits ConverterDescription::ConvertAttachmentSamples(const description::SampleCountFlagBits flags)
{
	return static_cast<VkSampleCountFlagBits>(flags);
}

VkDescriptorType ConverterDescription::ConvertUniformTypeToDescriptorType(const description::UniformType type)
{
	switch (type)
	{
	case description::UniformType::COMBINED_IMAGE_SAMPLER:
	{
		return VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	}
	case description::UniformType::UNIFORM_BUFFER:
	{
		return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}
	case description::UniformType::STORAGE_BUFFER:
	{
		return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	}
	case description::UniformType::STORAGE_IMAGE:
	{
		return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	}
	case description::UniformType::ACCELERATION_STRUCTURE:
	{
		return VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	}
	}

	LOGEXC(std::runtime_error, "[ConverterDescription::ConvertUniformTypeToDescriptorType] failed to convert uniform type to descriptor type");
	return VkDescriptorType::VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

VkVertexInputRate ConverterDescription::ConvertVertexInputRate(const description::VertexInputRate inputRate)
{
	return static_cast<VkVertexInputRate>(inputRate);
}

VkImageLayout ConverterDescription::ConvertImageLayout(const description::ImageLayout layout)
{
	return static_cast<VkImageLayout>(layout);
}

VkBufferUsageFlags ConverterDescription::ConvertUniformBufferType(const description::UniformBufferType type)
{
	switch (type)
	{
	case description::UniformBufferType::UNIFORM:
	{
		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}
	case description::UniformBufferType::STORAGE:
	{
		return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}
	}

	LOGEXC(std::runtime_error, "[ConverterDescription::ConvertUniformBufferType] failed to convert uniform buffer type to buffer usage flags");
	return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
}

description::UniformType ConverterDescription::ConvertUniformBufferTypeToUniformType(const description::UniformBufferType type)
{
	switch (type)
	{
	case description::UniformBufferType::STORAGE:
	{
		return description::UniformType::STORAGE_BUFFER;
	}
	case description::UniformBufferType::UNIFORM:
	{
		return description::UniformType::UNIFORM_BUFFER;
	}
	}

	LOGEXC(std::runtime_error, "[ConverterDescription::ConvertUniformBufferTypeToUniformType] failed to convert uniform buffer type to uniform type");
	return description::UniformType::NONE;
}

}
