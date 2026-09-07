#include "manager_shader_module.h"

#include "compiler_shader_module.h"
#include "creator_shader_module.h"
#include "reader_shader.h"
#include "../vulkan_manager_assets.h"
#include "../converter_description.h"

#include <guard_next_id.h>
#include <logger_instance.h>

#include <iostream>
#include <map>

namespace render
{

std::shared_ptr<DataShader> ManagerShaderModule::GetShaderData(const RenderId& id) const
{
	if (!id.IsValid())
	{
		LOGEXC(std::runtime_error, "[ManagerShaderModule::GetShaderData] don't have this id");
	}

	if (static_cast<uint64_t>(shaders_.size()) <= id.GetId())
	{
		LOGEXC(std::runtime_error, "[ManagerShaderModule::GetShaderData] invalid id value");
	}

	return shaders_[id];
}

std::shared_ptr<DataShaderModule> ManagerShaderModule::GetShaderModule(const RenderId& id) const
{
	return GetShaderData(id)->shader_module_data;
}

const DataShaderModuleReflection& ManagerShaderModule::GetShaderModuleReflectionData(const RenderId& id) const
{
	if (!id.IsValid())
	{
		LOGEXC(std::runtime_error, "[ManagerShaderModule::GetShaderModuleReflectionData] don't have this id");
	}

	if (static_cast<uint64_t>(shader_module_reflections_.size()) <= id.GetId())
	{
		LOGEXC(std::runtime_error, "[ManagerShaderModule::GetShaderModuleReflectionData] invalid id value");
	}

	return shader_module_reflections_[id];
}

RenderId ManagerShaderModule::FindOrAddShaderModule(
	const description::ShaderDescription& shaderDescription,
	std::shared_ptr<DataDevice> device,
	const std::filesystem::path& cacheDirectory)
{
	auto id = findShader(device, shaderDescription.filepath);

	if (id.IsValid())
	{
		shader_use_count_[id]++;
		return id;
	}

	id = addShader(
		device,
		shaderDescription,
		cacheDirectory);

	if (id.IsValid())
	{
		shader_use_count_[id]++;
		return id;
	}

	LOGEXC(std::runtime_error, "[ManagerShaderModule::FindOrAddShaderModule] failed to create shader, the id is invalid");
	return id;
}

void ManagerShaderModule::DeleteShaderModule(render::RenderId& shaderModuleId)
{
	if (--shader_use_count_[shaderModuleId] > 0)
	{
		shaderModuleId = GeneratorId::GenerateInvalidId();
		return;
	}

	auto shaderIdByFilepath = shader_ids_by_filepath_.find(shaders_[shaderModuleId]->filepath.native());
	if (shaderIdByFilepath != shader_ids_by_filepath_.end())
	{
		shaderIdByFilepath->second.erase(shaders_[shaderModuleId]->shader_module_data->device);
	}

	next_shader_id_= std::min(next_shader_id_, shaderModuleId.GetId());
	shaders_[shaderModuleId] = nullptr;
	shader_module_reflections_[shaderModuleId] = {};
	shaderModuleId = GeneratorId::GenerateInvalidId();
}

VkPipelineShaderStageCreateInfo ManagerShaderModule::CreatePipelineShaderStageCreateInfo(const RenderId& id) const
{
	auto shader = GetShaderData(id);
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = shader->stage;
	fragShaderStageInfo.module = shader->shader_module_data->shader;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr;

	return fragShaderStageInfo; 
}

const std::vector<VkVertexInputBindingDescription>& ManagerShaderModule::GetVertexInputBindingDescription(const RenderId& id) const
{
	return shaders_[id]->input_binding_descriptions;
}

const std::vector<VkVertexInputAttributeDescription>& ManagerShaderModule::GetVertexInputAttributeDescription(const RenderId& id) const
{
	return shaders_[id]->input_attribute_descriptions;
}

RenderId ManagerShaderModule::findShader(
	std::shared_ptr<DataDevice> device,
	const std::filesystem::path& filepath) const
{
	auto shaderIdByFilepath = shader_ids_by_filepath_.find(filepath.native());
	if (shaderIdByFilepath != shader_ids_by_filepath_.end())
	{
		auto shaderIdByDevice = shaderIdByFilepath->second.find(device);
		if (shaderIdByDevice != shaderIdByFilepath->second.end())
		{
			return shaderIdByDevice->second;
		}
	}

	return RenderId();
}

RenderId ManagerShaderModule::addShader(
	std::shared_ptr<DataDevice> device,
	const description::ShaderDescription& shaderDescription,
	const std::filesystem::path& cacheDirectory)
{
	auto shaderPath = ManagerAssetsVulkan::GetInterface()->GetAssetPath() / shaderDescription.filepath;
	std::shared_ptr<DataShader> shaderData(new DataShader());
	std::string spv = CompilerShaderModule::ReadCacheFile(shaderPath, cacheDirectory);
	if (spv.empty())
	{
		try
		{
			const std::string glsl = ReaderShader::ReadFile(shaderPath);
			spv = CompilerShaderModule::Compile(shaderPath.string(), glsl, shaderDescription.type);
			CompilerShaderModule::WriteCacheFile(shaderPath, cacheDirectory, spv);
		}
		catch (const std::exception& exception)
		{
			LOG(Loglvl::error, "[ManagerShaderModule::addShader]", exception.what());

			return GeneratorId::GenerateInvalidId();
		}
	}

	auto nextShaderId = GeneratorId::GenerateUniqueId<RenderId>(next_shader_id_);

	auto guardResize = utils::GuardResize::MayBeResize(
		next_shader_id_,
		[](const std::shared_ptr<DataShader>& shader) { return bool(!shader); },
		shaders_,
		shader_module_reflections_,
		shader_use_count_);

	const DataShaderModuleReflection dataShaderModuleReflection = CompilerShaderModule::Reflect(spv, shaderDescription.type);
	shader_module_reflections_[nextShaderId] = dataShaderModuleReflection;

	shaderData->filepath = shaderPath;
	shaderData->stage = ConverterDescription::ShaderTypeToShaderStage(shaderDescription.type);

	if (shaderData->stage == VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
	{
		shaderData->input_binding_descriptions = createInputBindingDescriptions(dataShaderModuleReflection, shaderDescription.binding_descriptions);
		shaderData->input_attribute_descriptions = createInputAttributeDescriptions(dataShaderModuleReflection, shaderDescription.binding_descriptions);
	}

	shaderData->shader_module_data = CreatorShaderModule::CreateShaderModule(device, spv);

	shaders_[nextShaderId] = shaderData;
	shader_ids_by_filepath_[shaderData->filepath.native()][device] = nextShaderId;
	return nextShaderId;
}

std::vector<VkVertexInputBindingDescription> ManagerShaderModule::createInputBindingDescriptions(
	const DataShaderModuleReflection& dataShaderModuleReflection,
	const std::vector<description::InputBindingDescription>& inputBindingDescription)
{
	std::vector<VkVertexInputBindingDescription> vkInputBindingDescriptions;

	for (description::InputBindingDescription binding : inputBindingDescription)
	{
		VkVertexInputBindingDescription vkInputBindingDescription{};
		vkInputBindingDescription.binding = binding.binding;
		vkInputBindingDescription.inputRate = ConverterDescription::ConvertVertexInputRate(binding.input_rate);
		
		uint32_t stride = 0;
		for (const std::string& name : binding.names)
		{
			auto stageInputByName = dataShaderModuleReflection.stage_inputs_by_name.find(name);
			if (stageInputByName == dataShaderModuleReflection.stage_inputs_by_name.end())
			{
				LOGEXC(std::runtime_error, "[ManagerShaderModule::createInputBindingDescriptions] Can't find an input stage with the name: " + name); 
			}

			const DataShaderModuleReflection::DataVariable& variable = stageInputByName->second.variable;
			stride += variable.column_size * variable.size;
		}

		vkInputBindingDescription.stride = stride;

		vkInputBindingDescriptions.push_back(vkInputBindingDescription);
	}

	return vkInputBindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> ManagerShaderModule::createInputAttributeDescriptions(
	const DataShaderModuleReflection& dataShaderModuleReflection,
	const std::vector<description::InputBindingDescription>& inputBindingDescription)
{
	std::vector<VkVertexInputAttributeDescription> vkInputAttributeDescriptions;

	std::map<uint32_t, std::vector<DataShaderModuleReflection::DataStage>> sortedStagesByBinding;

	for (const auto& binding : inputBindingDescription)
	{
		for (const std::string& name : binding.names)
		{
			auto inputByName = dataShaderModuleReflection.stage_inputs_by_name.find(name);
			if (inputByName == dataShaderModuleReflection.stage_inputs_by_name.end())
			{
				LOGEXC(std::runtime_error, "[ManagerShaderModule::createInputAttributeDescriptions] input name " + name + " doesn't exist in the shader");
			}

			sortedStagesByBinding[binding.binding].push_back(inputByName->second);
		}
	}

	for (const auto& [binding, stages] : sortedStagesByBinding)
	{
		uint32_t offset = 0;
		for (const DataShaderModuleReflection::DataStage& stage : stages)
		{
			for (uint32_t column = 0; column < stage.variable.column_size; column++)
			{
				VkVertexInputAttributeDescription& vertexInputAttributeDescription = vkInputAttributeDescriptions.emplace_back();
				vertexInputAttributeDescription.binding = binding;
				vertexInputAttributeDescription.location = stage.location + column;
				vertexInputAttributeDescription.format = ConverterDescription::FormatToVkFormat(stage.variable.format);
				vertexInputAttributeDescription.offset = offset;

				offset += stage.variable.size;
			}
		}
	}

	return vkInputAttributeDescriptions;
}

}
