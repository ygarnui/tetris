#pragma once


namespace render
{
	struct DetailShaderProgram;

	class ManagerShaderProgram
	{
	public:
		[[nodiscard]] virtual ShaderProgramId CreateShaderProgram(
			const GraphicsWindowId& windowId,
			const std::vector<description::ShaderDescription>& shaderDescriptions) = 0;

		virtual void deleteShaderProgram(ShaderProgramId& shaderProgramId) = 0;

		[[nodiscard]] virtual const std::vector<DataShaderModuleReflection>& GetShaderProgramReflectionData(
			const ShaderProgramId& shaderProgramId) const = 0;

		[[nodiscard]] virtual std::vector<VkPipelineShaderStageCreateInfo> CreatePipelineShaderStageCreateInfo(
			const ShaderProgramId& shaderProgramId) const = 0;

		[[nodiscard]] virtual const std::vector<VkVertexInputBindingDescription>& GetVertexInputBindingDescriptions(
			const ShaderProgramId& shaderProgramId) const = 0;

		[[nodiscard]] virtual const std::vector<VkVertexInputAttributeDescription>& GetVertexInputAttributeDescriptions(
			const ShaderProgramId& shaderProgramId) const = 0;

		[[nodiscard]] virtual std::vector<std::shared_ptr<DataDescriptorSetLayout>> GetDescriptorSetLayoutsData(
			const ShaderProgramId& shaderProgramId) const = 0;

		[[nodiscard]] virtual std::shared_ptr<DataDescriptorSetLayout> GetDescriptorSetLayoutData(
			const ShaderProgramId& shaderProgramId,
			const uint32_t descriptorSetIndex) const = 0;

		[[nodiscard]] virtual const DataShaderModuleReflection& GetShaderModuleReflectionData(
			const RenderId& shaderModuleId) const = 0;

		[[nodiscard]] virtual const DetailShaderProgram& GetDetailShaderProgram(const ShaderProgramId& shaderProgramId) const = 0;
	};
}
