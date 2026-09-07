#include "vulkan_manager_shader_program.h"

#include "../converter_description.h"
#include "../buffers/creator_descriptor_set_layout.h"

#include <logger_instance.h>
#include <guard_next_id.h>

#include "../manager_window.h"

namespace render
{
	VulkanManagerShaderProgram::VulkanManagerShaderProgram()
	{
	}
		
	std::shared_ptr<VulkanManagerShaderProgram>& VulkanManagerShaderProgram::Get()
	{
		static std::shared_ptr<VulkanManagerShaderProgram> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<VulkanManagerShaderProgram>(new VulkanManagerShaderProgram());
		}
		return manager;
	}

	VulkanManagerShaderProgram::~VulkanManagerShaderProgram()
	{
		LOG(Loglvl::debug, "[VulkanManagerShaderProgram::~VulkanManagerShaderProgram]");
	}

	ShaderProgramId VulkanManagerShaderProgram::CreateShaderProgram(
		const GraphicsWindowId& windowId,
		const std::vector<description::ShaderDescription>& shaderDescriptions)
	{
		return findOrAddShaderProgram(
			shaderDescriptions,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerAssetsVulkan::GetInterface()->GetShaderCachePath());
	}

	const std::vector<DataShaderModuleReflection>& VulkanManagerShaderProgram::GetShaderProgramReflectionData(
		const ShaderProgramId& shaderProgramId) const
	{
		return GetDetailShaderProgram(shaderProgramId).data_shader_module_reflections;
	}

	ShaderProgramId VulkanManagerShaderProgram::findOrAddShaderProgram(
		const std::vector<description::ShaderDescription>& shaderDescriptions,
		const LogicalDeviceId& logicalDeviceId,
		const std::filesystem::path& cacheDirectory)
	{
		auto id = findShaderProgram(shaderDescriptions, logicalDeviceId);

		if (id.IsValid())
		{
			shader_program_use_count_[id]++;
			return id;
		}

		id = addShaderProgram(
			shaderDescriptions,
			logicalDeviceId,
			cacheDirectory);

		if (!id.IsValid())
		{
			LOGEXC(std::runtime_error, "failed to create shader program, the id is invalid");
		}

		addDescriptorSetLayoutsToShader(id);

		shader_program_use_count_[id]++;

		return id;
	}

	void VulkanManagerShaderProgram::deleteShaderProgram(ShaderProgramId& shaderProgramId)
	{
		if (--shader_program_use_count_[shaderProgramId] > 0)
		{
			shaderProgramId = GeneratorId::GenerateInvalidId<ShaderProgramId>();
			return;
		}

		auto& detailShaderProgram = detail_shader_programs_[shaderProgramId];

		for (auto& [type, detailShaderModule] : detailShaderProgram.detail_shader_modules_by_type)
		{
			if (detailShaderModule.id.IsValid())
			{
				manager_shader_module_.DeleteShaderModule(detailShaderModule.id);
			}
		}

		detailShaderProgram = {};
		descriptor_set_layouts_by_shader_program_id_[shaderProgramId].clear();

		next_shader_program_id_ = std::min(next_shader_program_id_, shaderProgramId.GetId());
		next_descriptor_set_layout_id_ = std::min(next_descriptor_set_layout_id_, shaderProgramId.GetId());

		shaderProgramId = GeneratorId::GenerateInvalidId<ShaderProgramId>();
	}

	const DetailShaderProgram& VulkanManagerShaderProgram::GetDetailShaderProgram(const ShaderProgramId& shaderProgramId) const
	{
		if (!shaderProgramId.IsValid())
		{
			LOGEXC(std::runtime_error, "[VulkanManagerShaderProgram::GetDetailShaderProgram] don't have this id");
		}

		if (detail_shader_programs_.size() <= shaderProgramId)
		{
			LOGEXC(std::runtime_error, "[VulkanManagerShaderProgram::GetDetailShaderProgram] invalid id value");
		}

		return detail_shader_programs_[shaderProgramId];
	}

	std::vector<VkPipelineShaderStageCreateInfo> VulkanManagerShaderProgram::CreatePipelineShaderStageCreateInfo(const ShaderProgramId& shaderProgramId) const
	{
		const auto& program = detail_shader_programs_[shaderProgramId];
		std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfo;
		for (const auto& [type, detailShaderModule] : program.detail_shader_modules_by_type)
		{
			pipelineShaderStageCreateInfo.push_back(
				createPipelineShaderStageCreateInfo(
					detailShaderModule.stage,
					manager_shader_module_.GetShaderModule(detailShaderModule.id)->shader,
					name_main_func_));
		}
		
		return pipelineShaderStageCreateInfo;
	}

	const std::vector<VkVertexInputBindingDescription>& VulkanManagerShaderProgram::GetVertexInputBindingDescriptions(const ShaderProgramId& shaderProgramId) const
	{
		const auto& program = detail_shader_programs_[shaderProgramId];

		return manager_shader_module_.GetVertexInputBindingDescription(program.detail_shader_modules_by_type.at(description::ShaderType::VERTEX).id);
	}

	const std::vector<VkVertexInputAttributeDescription>& VulkanManagerShaderProgram::GetVertexInputAttributeDescriptions(const ShaderProgramId& shaderProgramId) const
	{
		const auto& program = detail_shader_programs_[shaderProgramId];

		return manager_shader_module_.GetVertexInputAttributeDescription(program.detail_shader_modules_by_type.at(description::ShaderType::VERTEX).id);
	}

	std::vector<std::shared_ptr<DataDescriptorSetLayout>> VulkanManagerShaderProgram::GetDescriptorSetLayoutsData(const ShaderProgramId& shaderProgramId) const
	{
		const auto& descriptorSetLayoutByShaderProgram = descriptor_set_layouts_by_shader_program_id_[shaderProgramId];

		std::vector<std::shared_ptr<DataDescriptorSetLayout>> descriptorSetLayoutsData;
		for (const auto& [descriptorSet, layout] : descriptorSetLayoutByShaderProgram)
		{
			descriptorSetLayoutsData.push_back(layout);
		}

		return descriptorSetLayoutsData;
	}

	std::shared_ptr<DataDescriptorSetLayout> VulkanManagerShaderProgram::GetDescriptorSetLayoutData(
		const ShaderProgramId& shaderProgramId,
		const uint32_t descriptorSetIndex) const
	{
		const auto& descriptorSetLayoutByShaderProgram = descriptor_set_layouts_by_shader_program_id_[shaderProgramId];
		auto descriptorSetLayout = descriptorSetLayoutByShaderProgram.find(descriptorSetIndex);
		if (descriptorSetLayout == descriptorSetLayoutByShaderProgram.end())
		{
			LOGEXC(std::runtime_error, "Shader program doesn't have descriptor set layout with set ", descriptorSetIndex);
		}
		return descriptorSetLayout->second;
	}

	const DataShaderModuleReflection& VulkanManagerShaderProgram::GetShaderModuleReflectionData(const RenderId& shaderModuleId) const
	{
		return manager_shader_module_.GetShaderModuleReflectionData(shaderModuleId);
	}

	ShaderProgramId VulkanManagerShaderProgram::findShaderProgram(
		const std::vector<description::ShaderDescription>& shaderDescriptions,
		const LogicalDeviceId& logicalDeviceId)
	{
		uint64_t shaderProgramIdRaw = 0;
		for (const auto& detail : detail_shader_programs_)
		{
			if (detail.logical_device_id != logicalDeviceId ||
				detail.detail_shader_modules_by_type.size() != shaderDescriptions.size())
			{
				break;
			}

			uint32_t foundShaders = 0;
			for (const auto& shaderDescription : shaderDescriptions)
			{
				auto detailShaderModuleByType = detail.detail_shader_modules_by_type.find(shaderDescription.type);
				if (detailShaderModuleByType != detail.detail_shader_modules_by_type.end())
				{
					if (shaderDescription.filepath == detailShaderModuleByType->second.filepath)
					{
						foundShaders++;
					}
					else
					{
						break;
					}
				}
			}

			if (foundShaders == shaderDescriptions.size())
			{
				return GeneratorId::GenerateUniqueId<ShaderProgramId>(shaderProgramIdRaw);
			}

			shaderProgramIdRaw++;
		}

		return GeneratorId::GenerateInvalidId<ShaderProgramId>();
	}

	ShaderProgramId VulkanManagerShaderProgram::addShaderProgram(
		const std::vector<description::ShaderDescription>& shaderDescriptions,
		const LogicalDeviceId& logicalDeviceId,
		const std::filesystem::path& cacheDirectory)
	{
		const std::string deviceName = ManagerDevice::Get()->GetVkPhysicalDeviceProperties(
			ManagerDevice::Get()->GetPhysicalDeviceIdByLogicalDeviceId(logicalDeviceId)).deviceName;
		const std::filesystem::path deviceCacheDirectory = cacheDirectory / deviceName;

		DetailShaderProgram shaderProgram{};
		shaderProgram.logical_device_id = logicalDeviceId;

		for (const auto& shaderDescription : shaderDescriptions)
		{
			DetailShaderModule shaderModule{};
			shaderModule.filepath = shaderDescription.filepath.string();
			shaderModule.stage = ConverterDescription::ShaderTypeToShaderStage(shaderDescription.type);
			shaderModule.id = manager_shader_module_.FindOrAddShaderModule(
				shaderDescription,
				ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
				deviceCacheDirectory);
			shaderProgram.detail_shader_modules_by_type.emplace(shaderDescription.type, shaderModule);
			shaderProgram.data_shader_module_reflections.emplace_back(manager_shader_module_.GetShaderModuleReflectionData(shaderModule.id));
		}

		auto nextShaderProgramId = GeneratorId::GenerateUniqueId<ShaderProgramId>(next_shader_program_id_);

		auto guardResize = utils::GuardResize::MayBeResize(
			next_shader_program_id_,
			[](const DetailShaderProgram& detail) { return !detail.logical_device_id.IsValid(); },
			detail_shader_programs_,
			shader_program_use_count_);

		detail_shader_programs_[nextShaderProgramId] = shaderProgram;

		return nextShaderProgramId;
	}

	void VulkanManagerShaderProgram::addDescriptorSetLayoutsToShader(const ShaderProgramId& shaderProgramId)
	{
		auto nextDescriptorSetLayoutId = GeneratorId::GenerateUniqueId<ShaderProgramId>(next_descriptor_set_layout_id_);

		auto guardResize = utils::GuardResize::MayBeResize(
			next_descriptor_set_layout_id_,
			[](const std::map<uint32_t, std::shared_ptr<DataDescriptorSetLayout>>& map) { return map.empty(); },
			descriptor_set_layouts_by_shader_program_id_);

		auto createVkDescriptorSetLayoutBinding = [](
			const DataShaderModuleReflection::DescriptorSetLayoutBinding& binding) -> VkDescriptorSetLayoutBinding
		{
			VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
			descriptorSetLayoutBinding.binding = binding.binding;
			descriptorSetLayoutBinding.descriptorCount = binding.count;
			descriptorSetLayoutBinding.descriptorType = ConverterDescription::ConvertUniformTypeToDescriptorType(binding.type);
			descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

			for (const auto& stage : binding.stages)
			{
				descriptorSetLayoutBinding.stageFlags |= ConverterDescription::ShaderTypeToShaderStage(stage);
			}

			return descriptorSetLayoutBinding;
		};

		auto& curProramDetail = detail_shader_programs_[shaderProgramId];
		if (curProramDetail.detail_shader_modules_by_type.size() > 2)
		{
			LOG(Loglvl::debug, "test");
		}

		std::map<uint32_t, std::map<uint32_t, DataShaderModuleReflection::DescriptorSetLayoutBinding>>
			bindingsByDescriptorSetData;

		std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindingsByDescriptorSet;
		for (const auto& [type, detailShaderModule] : curProramDetail.detail_shader_modules_by_type)
		{
			const DataShaderModuleReflection& dataReflectionVertex = manager_shader_module_.GetShaderModuleReflectionData(
				detailShaderModule.id);
			for (const auto& [descriptorSet, bindings] : dataReflectionVertex.bindings_by_descriptor_set)
			{
				for (const auto& binding : bindings)
				{
					auto& curMap = bindingsByDescriptorSetData[descriptorSet];
					if (curMap.count(binding.binding) == 0)
					{
						curMap[binding.binding] = binding;
					}
					else
					{
						for (auto stage : binding.stages)
						{
							curMap[binding.binding].stages.push_back(stage);
						}
					}
				}
			}
		}

		for (const auto& [descriptorSet, bindings] : bindingsByDescriptorSetData)
		{
			for (const auto& [_, binding] : bindings)
			{
				auto& curMap = bindingsByDescriptorSet[descriptorSet];
				curMap.push_back(createVkDescriptorSetLayoutBinding(binding));
			}
		}

		auto& descriptorSetLayouts = descriptor_set_layouts_by_shader_program_id_[nextDescriptorSetLayoutId];
		for (const auto& [descriptorSet, bindings] : bindingsByDescriptorSet)
		{
			descriptorSetLayouts[descriptorSet] = CreatorDescriptorSetLayout::CreateDescriptorSetLayout(
				bindings, ManagerDevice::Get()->GetLogicalDevice(detail_shader_programs_[shaderProgramId].logical_device_id));
		}
	}

	VkPipelineShaderStageCreateInfo VulkanManagerShaderProgram::createPipelineShaderStageCreateInfo(
		const VkShaderStageFlagBits& shaderStage,
		const VkShaderModule module,
		const std::string& funcName) const
	{
		VkPipelineShaderStageCreateInfo stageInfo{};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = shaderStage;
		stageInfo.module = module;
		stageInfo.pName = funcName.c_str();
		stageInfo.pSpecializationInfo = nullptr;

		return stageInfo;
	}

}
