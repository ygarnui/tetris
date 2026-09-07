#include "compiler_shader_module.h"

#include "shader_includer.h"

#include <logger_instance.h>
#include <shaderc/shaderc.hpp>

#include <fstream>
#include <iostream>
#include <map>

namespace render
{

std::string CompilerShaderModule::Compile(
	const std::string& debugShaderName,
	const std::string& code,
	const description::ShaderType type)
{
	shaderc_shader_kind kind{};
	switch (type)
	{
	case description::ShaderType::VERTEX:
	{
		kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
		break;
	}
	case description::ShaderType::GEOMETRY:
	{
		kind = shaderc_shader_kind::shaderc_glsl_geometry_shader;
		break;
	}
	case description::ShaderType::COMPUTE:
	{
		kind = shaderc_shader_kind::shaderc_glsl_compute_shader;
		break;
	}
	case description::ShaderType::TESSELLATION_CONTROL:
	{
		kind = shaderc_shader_kind::shaderc_glsl_tess_control_shader;
		break;
	}
	case description::ShaderType::TESSELLATION_EVALUATION:
	{
		kind = shaderc_shader_kind::shaderc_glsl_tess_evaluation_shader;
		break;
	}
	case description::ShaderType::FRAGMENT:
	{
		kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
		break;
	}
	case description::ShaderType::RAY_GENERATION:
	{
		kind = shaderc_shader_kind::shaderc_glsl_raygen_shader;
		break;
	}
	case description::ShaderType::RAY_MISS:
	{
		kind = shaderc_shader_kind::shaderc_glsl_miss_shader;
		break;
	}
	case description::ShaderType::RAY_CLOSEST_HIT:
	{
		kind = shaderc_shader_kind::shaderc_glsl_closesthit_shader;
		break;
	}
	case description::ShaderType::RAY_ANY_HIT:
	{
		kind = shaderc_shader_kind::shaderc_glsl_anyhit_shader;
		break;
	}
	case description::ShaderType::RAY_INTERSECTION:
	{
		kind = shaderc_shader_kind::shaderc_glsl_intersection_shader;
		break;
	}
	case description::ShaderType::NONE:
	{
		LOGEXC(std::runtime_error, "[CompilerShaderModule::Compile] shader type is undefined");
		break;
	}
	}

	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	options.SetOptimizationLevel(shaderc_optimization_level_zero);
	options.SetGenerateDebugInfo();
	options.SetIncluder(std::make_unique<ShaderIncluder>());
	options.SetPreserveBindings(true);

	// GL_EXT_ray_tracing needs SPIR-V 1.4, which in turn needs a Vulkan 1.2 target environment.
	// Without this the compiler defaults to Vulkan 1.0 and rejects every ray tracing shader.
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
	options.SetTargetSpirv(shaderc_spirv_version_1_4);

	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(code, kind, debugShaderName.c_str(), options);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		LOGEXC(std::runtime_error, result.GetErrorMessage());
	}

	const std::string spv((const char*)result.cbegin(), (const char*)result.cend());

	LOG(Loglvl::info, "[CompilerShaderModule::Compile] Shader has been compiled: ", debugShaderName);

	return spv;
}

std::string CompilerShaderModule::ReadCacheFile(
	const std::filesystem::path& shaderFilepath,
	const std::filesystem::path& cacheDirectory)
{
	if (shaderFilepath.empty())
	{
		return {};
	}

	const std::string cacheFilename = shaderFilepath.filename().string() + ".cache";
	const std::filesystem::path cacheFilepath = cacheDirectory / cacheFilename;

	std::ifstream file(cacheFilepath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		return {};
	}

	const size_t fileSize = (size_t)file.tellg();
	const size_t timeSize = sizeof(size_t);

	if (fileSize <= timeSize)
	{
		return {};
	}

	const size_t shaderLastModifiedTime = std::filesystem::last_write_time(shaderFilepath).time_since_epoch().count();
	size_t shaderLastModifiedSavedTime;

	file.seekg(0);
	file.read((char*)&shaderLastModifiedSavedTime, timeSize);

	if (shaderLastModifiedTime != shaderLastModifiedSavedTime)
	{
		file.close();
		return {};
	}

	std::string buffer;
	buffer.resize(fileSize - timeSize);

	file.read(buffer.data(), fileSize);
	file.close();

	LOG(Loglvl::info, "[CompilerShaderModule::Compile] Shader cache has been loaded: " , shaderFilepath);

	return buffer;
}

void CompilerShaderModule::WriteCacheFile(
	const std::filesystem::path& shaderFilepath,
	const std::filesystem::path& cacheDirectory,
	const std::string& spv)
{
	if (spv.empty() || shaderFilepath.empty() || !std::filesystem::exists(shaderFilepath))
	{
		LOGEXC(std::runtime_error, "[CompilerShaderModule::WriteCacheFile] failed to write shader cache by filepath: " + shaderFilepath.string());
	}

	if (!std::filesystem::exists(cacheDirectory))
	{
		std::filesystem::create_directories(cacheDirectory);
	}

	const std::string cacheFilename = shaderFilepath.filename().string() + ".cache";
	const std::filesystem::path cacheFilepath = cacheDirectory / cacheFilename;

	std::ofstream file(cacheFilepath, std::ios::binary);

	const size_t shaderLastModifiedTime = std::filesystem::last_write_time(shaderFilepath).time_since_epoch().count();

	file.write((char*)&shaderLastModifiedTime, sizeof(size_t));
	file.write(spv.data(), spv.size());
	file.close();
}

DataShaderModuleReflection CompilerShaderModule::Reflect(const std::string& spv, const description::ShaderType type)
{
	std::vector<uint32_t> convertedSpv;
	convertedSpv.resize(spv.size() / sizeof(uint32_t));
	memcpy(convertedSpv.data(), spv.data(), spv.size());

	DataShaderModuleReflection dataReflection{};

	spirv_cross::CompilerReflection compiler(std::move(convertedSpv));
	spirv_cross::ShaderResources shaderResources = compiler.get_shader_resources();

	auto reflectImage = [](
		const spirv_cross::CompilerReflection& compiler,
		const description::UniformType type,
		const spirv_cross::SmallVector<spirv_cross::Resource>& stageVariables,
		std::unordered_map<std::string, DataShaderModuleReflection::DataImage>& imageByName)
	{
		for (const spirv_cross::Resource& sampledImage : stageVariables)
		{
			DataShaderModuleReflection::ResourceLocation resourceLocation{};
			const spirv_cross::SPIRType& spirType = compiler.get_type(sampledImage.type_id);
			resourceLocation.descriptor_set = compiler.get_decoration(sampledImage.id, spv::DecorationDescriptorSet);
			resourceLocation.binding = compiler.get_decoration(sampledImage.id, spv::DecorationBinding);

			DataShaderModuleReflection::DataImage dataImage{};
			dataImage.resource_location = resourceLocation;
			dataImage.type = type;
			if (spirType.array.size() == 1)
			{
				dataImage.count = spirType.array[0];
			}

			imageByName[sampledImage.name] = dataImage;
		}
	};

	reflectImage(compiler, description::UniformType::COMBINED_IMAGE_SAMPLER, shaderResources.sampled_images, dataReflection.sampled_images_by_name);
	reflectImage(compiler, description::UniformType::STORAGE_IMAGE, shaderResources.storage_images, dataReflection.sampled_images_by_name);
	// An acceleration structure is neither an image nor a buffer, but it is reflected the same way:
	// only its set and binding matter, and that is all this map is used for downstream.
	reflectImage(compiler, description::UniformType::ACCELERATION_STRUCTURE, shaderResources.acceleration_structures, dataReflection.sampled_images_by_name);

	std::function<void(
		const spirv_cross::CompilerReflection&,
		const spirv_cross::SPIRType&,
		std::unordered_map<std::string, DataShaderModuleReflection::DataVariable>&)> reflectBuffer;

	reflectBuffer = [&reflectBuffer](
		const spirv_cross::CompilerReflection& compiler,
		const spirv_cross::SPIRType& type,
		std::unordered_map<std::string, DataShaderModuleReflection::DataVariable>& variablesByName)
	{
		uint32_t memberCount = static_cast<uint32_t>(type.member_types.size());
		for (uint32_t memberIndex = 0; memberIndex < memberCount; memberIndex++)
		{
			DataShaderModuleReflection::DataVariable variable{};

			const spirv_cross::SPIRType& memberType = compiler.get_type(type.member_types[memberIndex]);

			if (memberType.basetype == spirv_cross::SPIRType::BaseType::Struct)
			{
				variable.size = static_cast<uint32_t>(compiler.get_declared_struct_size(type));
				variable.offset = compiler.type_struct_member_offset(type, memberIndex);
				reflectBuffer(compiler, memberType, variable.variables_by_name);
			}
			else
			{
				variable.column_size = memberType.columns;
				variable.format = convertSpirTypeToCoordsType(memberType);
				variable.offset = compiler.type_struct_member_offset(type, memberIndex);

				uint32_t numInVec = 1;
				if (!memberType.array.empty())
				{
					numInVec = memberType.array[memberIndex];
				}
				constexpr uint32_t bitsInByte = 8;
				variable.size = (memberType.width / bitsInByte) * memberType.vecsize * numInVec;
			}

			const std::string& memberName = compiler.get_member_name(type.self, memberIndex);
			variablesByName[memberName] = variable;
		}
	};

	auto reflectBuffers = [reflectBuffer](
		const spirv_cross::CompilerReflection& compiler,
		const description::UniformBufferType type,
		const spirv_cross::SmallVector<spirv_cross::Resource>& buffers,
		std::unordered_map<std::string, DataShaderModuleReflection::DataBuffer>& buffersByName)
	{
		for (const spirv_cross::Resource& uniformBuffer : buffers)
		{
			DataShaderModuleReflection::DataBuffer& dataBuffer = buffersByName[uniformBuffer.name];

			dataBuffer.type = type;
			dataBuffer.resource_location.descriptor_set = compiler.get_decoration(uniformBuffer.id, spv::Decoration::DecorationDescriptorSet);
			dataBuffer.resource_location.binding = compiler.get_decoration(uniformBuffer.id, spv::Decoration::DecorationBinding);

			const spirv_cross::SPIRType& type = compiler.get_type(uniformBuffer.base_type_id);
			dataBuffer.size = static_cast<uint32_t>(compiler.get_declared_struct_size(type));
			reflectBuffer(compiler, type, dataBuffer.variables_by_name);
		}
	};

	reflectBuffers(compiler, description::UniformBufferType::UNIFORM, shaderResources.uniform_buffers, dataReflection.uniform_buffers_by_name);
	reflectBuffers(compiler, description::UniformBufferType::STORAGE, shaderResources.storage_buffers, dataReflection.uniform_buffers_by_name);

	auto reflectStages = [](
		const spirv_cross::CompilerReflection& compiler,
		const spirv_cross::SmallVector<spirv_cross::Resource>& stageVariables,
		std::unordered_map<std::string, DataShaderModuleReflection::DataStage>& stagesByName)
	{
		std::map<uint32_t, std::string> stagesNamesByLocation;

		for (const spirv_cross::Resource& stageVariable : stageVariables)
		{
			DataShaderModuleReflection::DataStage& stageByName = stagesByName[stageVariable.name];
			stageByName.location = compiler.get_decoration(stageVariable.id, spv::DecorationLocation);

			DataShaderModuleReflection::DataVariable variable{};
			const spirv_cross::SPIRType& type = compiler.get_type(stageVariable.base_type_id);
			variable.column_size = type.columns;
			
			variable.format = convertSpirTypeToCoordsType(type);
			
			const uint32_t bitsInByte = 8;
			variable.size = (type.width / bitsInByte) * type.vecsize;

			stageByName.variable = variable;

			stagesNamesByLocation[stageByName.location] = stageVariable.name;
		}

		uint32_t offset = 0;
		for (auto& [location, name] : stagesNamesByLocation)
		{
			DataShaderModuleReflection::DataStage& stageByName = stagesByName.at(name);
			stageByName.variable.offset = offset;
			offset += stageByName.variable.column_size * stageByName.variable.size;
		}
	};

	reflectStages(compiler, shaderResources.stage_inputs, dataReflection.stage_inputs_by_name);
	reflectStages(compiler, shaderResources.stage_outputs, dataReflection.stage_outputs_by_name);

	auto createBinding = [type](
		const uint32_t binding,
		const uint32_t count,
		const description::UniformType uniformType)
	{
		DataShaderModuleReflection::DescriptorSetLayoutBinding layoutbinding{};
		layoutbinding.binding = binding;
		layoutbinding.count = count;
		layoutbinding.type = uniformType;
		layoutbinding.stages = { type };

		return layoutbinding;
	};

	// The resources are collected from unordered maps, so a binding has to be found by its
	// own number: using the position in the vector assumes the bindings arrive in order and
	// silently drops the ones that do not.
	auto addOrMergeBinding = [&dataReflection, &createBinding, type](
		const uint32_t descriptorSet,
		const uint32_t binding,
		const uint32_t count,
		const description::UniformType uniformType)
	{
		auto& bindings = dataReflection.bindings_by_descriptor_set[descriptorSet];

		for (auto& existing : bindings)
		{
			if (existing.binding == binding)
			{
				existing.stages.push_back(type);
				return;
			}
		}

		bindings.push_back(createBinding(binding, count, uniformType));
	};

	for (const auto& [name, sampledImage] : dataReflection.sampled_images_by_name)
	{
		addOrMergeBinding(
			sampledImage.resource_location.descriptor_set,
			sampledImage.resource_location.binding,
			sampledImage.count,
			sampledImage.type);
	}

	// Because uniform buffer arrays are not supported yet.
	const uint32_t defaultUniformBufferCount = 1;

	for (const auto& [name, uniformBuffer] : dataReflection.uniform_buffers_by_name)
	{
		addOrMergeBinding(
			uniformBuffer.resource_location.descriptor_set,
			uniformBuffer.resource_location.binding,
			defaultUniformBufferCount,
			ConverterDescription::ConvertUniformBufferTypeToUniformType(uniformBuffer.type));
	}

	// Need to sort the bindings by their location, because of the future indexing into this array by the binding location.
	for (auto& [set, bindings] : dataReflection.bindings_by_descriptor_set)
	{
		std::sort(bindings.begin(), bindings.end(), [](
			const DataShaderModuleReflection::DescriptorSetLayoutBinding& left,
			const DataShaderModuleReflection::DescriptorSetLayoutBinding& right)
			{
				return left.binding < right.binding;
			});
	}

	return dataReflection;
}

description::Format CompilerShaderModule::convertSpirTypeToCoordsType(const spirv_cross::SPIRType type)
{
	switch (type.width)
	{
	case 8:
	{
		return convertSpirTypeToCoordsType8Bits(type);
	}
	case 16:
	{
		return convertSpirTypeToCoordsType16Bits(type);
	}
	case 32:
	{
		return convertSpirTypeToCoordsType32Bits(type);
	}
	default:
	{
		return description::Format::UNDEFINED;
	}
	}
}

description::Format CompilerShaderModule::convertSpirTypeToCoordsType8Bits(const spirv_cross::SPIRType type)
{
	if (type.vecsize == 1)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::BaseType::Float:
		{
			return description::Format::UNDEFINED;
		}
		case spirv_cross::SPIRType::BaseType::Int:
		{
			return description::Format::R8_SINT;
		}
		case spirv_cross::SPIRType::BaseType::UInt:
		{
			return description::Format::R8_UINT;
		}
		default:
		{
			return description::Format::UNDEFINED;
		}
		}
	}

	if (type.vecsize == 2)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::BaseType::Float:
		{
			return description::Format::UNDEFINED;
		}
		case spirv_cross::SPIRType::BaseType::Int:
		{
			return description::Format::R8G8_SINT;
		}
		case spirv_cross::SPIRType::BaseType::UInt:
		{
			return description::Format::R8G8_UINT;
		}
		default:
			return description::Format::UNDEFINED;
		}
	}

	if (type.vecsize == 3)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::BaseType::Float:
		{
			return description::Format::UNDEFINED;
		}
		case spirv_cross::SPIRType::BaseType::Int:
		{
			return description::Format::R8G8B8_SINT;
		}
		case spirv_cross::SPIRType::BaseType::UInt:
		{
			return description::Format::R8G8B8_UINT;
		}
		default:
			return description::Format::UNDEFINED;
		}
	}

	if (type.vecsize == 4)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::BaseType::Float:
		{
			return description::Format::UNDEFINED;
		}
		case spirv_cross::SPIRType::BaseType::Int:
		{
			return description::Format::R8G8B8A8_SINT;
		}
		case spirv_cross::SPIRType::BaseType::UInt:
		{
			return description::Format::R8G8B8A8_UINT;
		}
		default:
			return description::Format::UNDEFINED;
		}
	}

	return description::Format::UNDEFINED;
}

description::Format CompilerShaderModule::convertSpirTypeToCoordsType16Bits(const spirv_cross::SPIRType type)
{
	if (type.vecsize == 1)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::BaseType::Float:
		{
			return description::Format::R16_SFLOAT;
		}
		case spirv_cross::SPIRType::BaseType::Int:
		{
			return description::Format::R16_SINT;
		}
		case spirv_cross::SPIRType::BaseType::UInt:
		{
			return description::Format::R16_UINT;
		}
		default:
		{
			return description::Format::UNDEFINED;
		}
		}
	}

	if (type.vecsize == 2)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::BaseType::Float:
		{
			return description::Format::R16G16_SFLOAT;
		}
		case spirv_cross::SPIRType::BaseType::Int:
		{
			return description::Format::R16G16_SINT;
		}
		case spirv_cross::SPIRType::BaseType::UInt:
		{
			return description::Format::R16G16_UINT;
		}
		default:
			return description::Format::UNDEFINED;
		}
	}

	if (type.vecsize == 3)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::BaseType::Float:
		{
			return description::Format::R16G16B16_SFLOAT;
		}
		case spirv_cross::SPIRType::BaseType::Int:
		{
			return description::Format::R16G16B16_SINT;
		}
		case spirv_cross::SPIRType::BaseType::UInt:
		{
			return description::Format::R16G16B16_UINT;
		}
		default:
			return description::Format::UNDEFINED;
		}
	}

	if (type.vecsize == 4)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::BaseType::Float:
		{
			return description::Format::R16G16B16A16_SFLOAT;
		}
		case spirv_cross::SPIRType::BaseType::Int:
		{
			return description::Format::R16G16B16A16_SINT;
		}
		case spirv_cross::SPIRType::BaseType::UInt:
		{
			return description::Format::R16G16B16A16_UINT;
		}
		default:
			return description::Format::UNDEFINED;
		}
	}

	return description::Format::UNDEFINED;
}

description::Format CompilerShaderModule::convertSpirTypeToCoordsType32Bits(const spirv_cross::SPIRType type)
{
	if (type.vecsize == 1)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::BaseType::Float:
		{
			return description::Format::R32_SFLOAT;
		}
		case spirv_cross::SPIRType::BaseType::Int:
		{
			return description::Format::R32_SINT;
		}
		case spirv_cross::SPIRType::BaseType::UInt:
		{
			return description::Format::R32_UINT;
		}
		default:
		{
			return description::Format::UNDEFINED;
		}
		}
	}

	if (type.vecsize == 2)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::BaseType::Float:
		{
			return description::Format::R32G32_SFLOAT;
		}
		case spirv_cross::SPIRType::BaseType::Int:
		{
			return description::Format::R32G32_SINT;
		}
		case spirv_cross::SPIRType::BaseType::UInt:
		{
			return description::Format::R32G32_UINT;
		}
		default:
			return description::Format::UNDEFINED;
		}
	}

	if (type.vecsize == 3)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::BaseType::Float:
		{
			return description::Format::R32G32B32_SFLOAT;
		}
		case spirv_cross::SPIRType::BaseType::Int:
		{
			return description::Format::R32G32B32_SINT;
		}
		case spirv_cross::SPIRType::BaseType::UInt:
		{
			return description::Format::R32G32B32_UINT;
		}
		default:
			return description::Format::UNDEFINED;
		}
	}

	if (type.vecsize == 4)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::BaseType::Float:
		{
			return description::Format::R32G32B32A32_SFLOAT;
		}
		case spirv_cross::SPIRType::BaseType::Int:
		{
			return description::Format::R32G32B32A32_SINT;
		}
		case spirv_cross::SPIRType::BaseType::UInt:
		{
			return description::Format::R32G32B32A32_UINT;
		}
		default:
			return description::Format::UNDEFINED;
		}
	}

	return description::Format::UNDEFINED;
}

}
