#pragma once

#include "manager_shader_module.h"
#include "manager_shader_program.h"
#include "../manager_device.h"
#include "../src/manager_window.h"
#include "../src/vulkan_manager_assets.h"
#include "../manager_base.h"

#include <buffer_description.h>
#include <graphics_id.h>

namespace render
{

struct DetailShaderModule
{
	RenderId id;
	std::filesystem::path filepath;
	VkShaderStageFlagBits stage;
};

struct DetailShaderProgram
{
	std::map<description::ShaderType, DetailShaderModule> detail_shader_modules_by_type;
	std::vector<DataShaderModuleReflection> data_shader_module_reflections;

	LogicalDeviceId logical_device_id = GeneratorId::GenerateInvalidId<LogicalDeviceId>();
};

class VulkanManagerShaderProgram : public ManagerShaderProgram, public ManagerBase
{
public:
	static std::shared_ptr<VulkanManagerShaderProgram>& Get();

	~VulkanManagerShaderProgram();

	VulkanManagerShaderProgram(const VulkanManagerShaderProgram&) = delete;
	VulkanManagerShaderProgram(VulkanManagerShaderProgram&&) = delete;

	VulkanManagerShaderProgram& operator= (const VulkanManagerShaderProgram&) = delete;
	VulkanManagerShaderProgram& operator= (VulkanManagerShaderProgram&&) = delete;

	[[nodiscard]] ShaderProgramId CreateShaderProgram(
		const GraphicsWindowId& windowId,
		const std::vector<description::ShaderDescription>& shaderDescriptions) override;

	[[nodiscard]] const std::vector<DataShaderModuleReflection>& GetShaderProgramReflectionData(
		const ShaderProgramId& shaderProgramId) const override;

	[[nodiscard]] std::vector<VkPipelineShaderStageCreateInfo> CreatePipelineShaderStageCreateInfo(
		const ShaderProgramId& shaderProgramId) const override;

	[[nodiscard]] const std::vector<VkVertexInputBindingDescription>& GetVertexInputBindingDescriptions(
		const ShaderProgramId& shaderProgramId) const override;

	[[nodiscard]] const std::vector<VkVertexInputAttributeDescription>& GetVertexInputAttributeDescriptions(
		const ShaderProgramId& shaderProgramId) const override;

	[[nodiscard]] std::vector<std::shared_ptr<DataDescriptorSetLayout>> GetDescriptorSetLayoutsData(
		const ShaderProgramId& shaderProgramId) const override;

	[[nodiscard]] std::shared_ptr<DataDescriptorSetLayout> GetDescriptorSetLayoutData(
		const ShaderProgramId& shaderProgramId,
		const uint32_t descriptorSetIndex) const override;

	[[nodiscard]] const DataShaderModuleReflection& GetShaderModuleReflectionData(const RenderId& shaderModuleId) const override;

	[[nodiscard]] const DetailShaderProgram& GetDetailShaderProgram(const ShaderProgramId& shaderProgramId) const override;

private:
	VulkanManagerShaderProgram();

	[[nodiscard]] ShaderProgramId findOrAddShaderProgram(
		const std::vector<description::ShaderDescription>& shaderDescriptions,
		const LogicalDeviceId& logicalDeviceId,
		const std::filesystem::path& cacheDirectory);

	void deleteShaderProgram(ShaderProgramId& shaderProgramId) override;

	[[nodiscard]] ShaderProgramId findShaderProgram(
		const std::vector<description::ShaderDescription>& shaderDescriptions,
		const LogicalDeviceId& logicalDeviceId);

	[[nodiscard]] ShaderProgramId addShaderProgram(
		const std::vector<description::ShaderDescription>& shaderDescriptions,
		const LogicalDeviceId& logicalDeviceId,
		const std::filesystem::path& cacheDirectory);

	void addDescriptorSetLayoutsToShader(const ShaderProgramId& shaderProgramId);

	// TO DO move in creator
	VkPipelineShaderStageCreateInfo createPipelineShaderStageCreateInfo(
		const VkShaderStageFlagBits& shaderStage,
		const VkShaderModule module,
		const std::string& funcName) const;

	ManagerShaderModule manager_shader_module_;

	std::string name_main_func_ = "main";

	size_t next_shader_program_id_ = 0;
	size_t next_descriptor_set_layout_id_ = 0;

	Vector<DetailShaderProgram, ShaderProgramId> detail_shader_programs_;
	Vector<uint32_t, ShaderProgramId> shader_program_use_count_;

	Vector<std::map<uint32_t, std::shared_ptr<DataDescriptorSetLayout>>, ShaderProgramId> descriptor_set_layouts_by_shader_program_id_;
};
}
