#pragma once

#include "buffer_description.h"
#include "format.h"

#include <unordered_map>
#include <vector>

/*!
\brief Reflection for GLSL shaders.
*/
struct DataShaderModuleReflection
{
	/*!
	\brief Shader uniform resource location.
	*/
	struct ResourceLocation
	{
		uint32_t descriptor_set;
		uint32_t binding;
	};

	/*!
	\brief Shader variable data.
	*/
	struct DataVariable
	{
		description::Format format;
		uint32_t column_size;
		uint32_t offset;
		uint32_t size;

		std::unordered_map<std::string, DataVariable> variables_by_name;
	};

	/*!
	\brief Shader sampled image data.
	*/
	struct DataImage
	{
		ResourceLocation resource_location;
		uint32_t count = 1;
		description::UniformType type;
	};

	/*!
	\brief Shader uniform/storage buffer data.
	*/
	struct DataBuffer
	{
		ResourceLocation resource_location;
		std::unordered_map<std::string, DataVariable> variables_by_name;
		uint32_t size;
		description::UniformBufferType type;
	};

	/*!
	\brief Shader input/output data for stages.
	*/
	struct DataStage
	{
		uint32_t location;
		DataVariable variable;
	};

	std::unordered_map<std::string, DataImage> sampled_images_by_name;
	std::unordered_map<std::string, DataBuffer> uniform_buffers_by_name;
	std::unordered_map<std::string, DataStage> stage_inputs_by_name;
	std::unordered_map<std::string, DataStage> stage_outputs_by_name;

	struct DescriptorSetLayoutBinding
	{
		uint32_t binding = 0;
		uint32_t count = 1;
		description::UniformType type = description::UniformType::UNIFORM_BUFFER;
		std::vector<description::ShaderType> stages;
	};

	std::unordered_map<uint32_t, std::vector<DescriptorSetLayoutBinding>> bindings_by_descriptor_set;
};
