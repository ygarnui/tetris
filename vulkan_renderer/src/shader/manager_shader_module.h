#pragma once

#include "../struct_data.h"
#include <render_base.h>


#include <vector.h>

#include <filesystem>

namespace render
{

class VulkanRenderBase;

class ManagerShaderModule
{
public:

	ManagerShaderModule() = default;

	ManagerShaderModule(const ManagerShaderModule&) = delete;
	ManagerShaderModule(ManagerShaderModule&&) = delete;

	ManagerShaderModule& operator= (const ManagerShaderModule&) = delete;
	ManagerShaderModule& operator= (ManagerShaderModule&&) = delete;

	[[nodiscard]] std::shared_ptr<DataShader> GetShaderData(const render::RenderId& id) const;

	[[nodiscard]] std::shared_ptr<DataShaderModule> GetShaderModule(const render::RenderId& id) const;
	
	[[nodiscard]] const DataShaderModuleReflection& GetShaderModuleReflectionData(const render::RenderId& id) const;

	[[nodiscard]] render::RenderId FindOrAddShaderModule(
		const description::ShaderDescription& shaderDescription,
		std::shared_ptr<DataDevice> device,
		const std::filesystem::path& cacheDirectory);

	void DeleteShaderModule(render::RenderId& shaderModuleId);

	[[nodiscard]] VkPipelineShaderStageCreateInfo CreatePipelineShaderStageCreateInfo(const render::RenderId& id) const;

	[[nodiscard]] const std::vector<VkVertexInputBindingDescription>& GetVertexInputBindingDescription(const render::RenderId& id) const;

	[[nodiscard]] const std::vector<VkVertexInputAttributeDescription>& GetVertexInputAttributeDescription(const render::RenderId& id) const;

private:

	[[nodiscard]] render::RenderId findShader(
		std::shared_ptr<DataDevice> device,
		const std::filesystem::path& filepath) const;

	[[nodiscard]] render::RenderId addShader(
		std::shared_ptr<DataDevice> device,
		const description::ShaderDescription& shaderDescription,
		const std::filesystem::path& cacheDirectory);

	std::vector<VkVertexInputBindingDescription> createInputBindingDescriptions(
		const DataShaderModuleReflection& dataShaderModuleReflection,
		const std::vector<description::InputBindingDescription>& inputBindingDescription);

	std::vector<VkVertexInputAttributeDescription> createInputAttributeDescriptions(
		const DataShaderModuleReflection& dataShaderModuleReflection,
		const std::vector<description::InputBindingDescription>& inputBindingDescription);

	size_t next_shader_id_ = 0;

	Vector<std::shared_ptr<DataShader>, RenderId> shaders_;
	Vector<DataShaderModuleReflection, RenderId> shader_module_reflections_;
	Vector<uint32_t, RenderId> shader_use_count_;
	std::unordered_map<std::filesystem::path::string_type, std::unordered_map<std::shared_ptr<DataDevice>, RenderId>> shader_ids_by_filepath_;

	friend class VulkanRenderBase;
};

}
