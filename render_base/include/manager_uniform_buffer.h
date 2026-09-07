#pragma once

#include "graphics_id.h"
#include "manager_buffer.h"


namespace render
{
	class ManagerUniformBuffer
	{
	public:
		virtual void UpdateSwapchainUniformBuffers(const SwapchainId& swapchainId, size_t imageIndex) = 0;

		virtual void DeleteUniformDescriptorSet(const DescriptorSetId& descriptorSetId) = 0;

		[[nodiscard]] virtual DescriptorSetId CreateUniformDescriptorSet(
			const GraphicsWindowId& windowId,
			const ShaderProgramId& shaderProgramId,
			const uint32_t descriptorSetIndex) = 0;

		[[nodiscard]] virtual UniformBufferId CreateUniformBuffer(
			const GraphicsWindowId& windowId,
			const description::UniformBufferType type,
			const uint64_t size) = 0;

		virtual std::vector<std::string> WriteToDescriptorSet(
			const DescriptorSetId& descriptorSetId,
			const std::vector<std::pair<std::string,UniformBufferId>>& uniformBuffers,
			const std::vector<std::pair<std::string,std::vector<std::vector<TextureId>>>>& textures) = 0;

		virtual void DeleteUniformBuffer(const UniformBufferId& uniformBufferId) = 0;

		virtual void WriteToUniformBuffer(
			const UniformBufferId& uniformBufferId,
			const void* data,
			const uint64_t size,
			const uint64_t offset) = 0;

		[[nodiscard]] virtual const std::vector<std::shared_ptr<DataUniformBuffer>>& GetUniformBuffer(
			const UniformBufferId& id) const = 0;

		[[nodiscard]] virtual const std::vector<VkDescriptorSet>& GetUniformDescriptorSet(
			const DescriptorSetId& id) const = 0;

		[[nodiscard]] virtual bool CanBeUsedUniformBufferWithFollowingParameters(
			const UniformBufferId& uniformBufferId,
			const GraphicsWindowId& windowId) const = 0;

		[[nodiscard]] virtual bool CanBeUsedDescriptorSetWithFollowingParameters(
			const DescriptorSetId& descriptorSetId,
			const GraphicsWindowId& windowId,
			const ShaderProgramId& shaderProgramId) const = 0;

		[[nodiscard]] virtual std::shared_ptr<DataDescriptorPool> GetDescriptorPoolBySwapchain(
			const SwapchainId& swapchainId) const = 0;
	};
}
