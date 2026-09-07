#pragma once

#include "../struct_data.h"
#include "../textures/vulkan_manager_textures.h"
#include "../shader/vulkan_manager_shader_program.h"
#include "../manager_device.h"
#include "../manager_swapchain.h"
#include "../manager_base.h"

#include <vector.h>

#include <set>

#include "manager_uniform_buffer.h"

namespace render
{

struct DetailUniformBuffer
{
	RenderId logical_device_id;
	// unused parameter, only for info
	SwapchainId swapchain_id;
	RenderId physical_device_id;
};

struct DetailDescriptorSet
{
	LogicalDeviceId logical_device_id;
	ShaderProgramId shader_program_id;
	// unused parameter, only for info
	SwapchainId swapchain_id;

	/*!
	\brief Descriptor set index in shader program by shader_program_id.
	*/
	uint32_t descriptor_set_index;
};

class VulkanManagerUniformBuffer  : public ManagerUniformBuffer, public ManagerBase
{
public:
	static std::shared_ptr<VulkanManagerUniformBuffer>& Get();

	~VulkanManagerUniformBuffer();

	VulkanManagerUniformBuffer(const VulkanManagerUniformBuffer&) = delete;
	VulkanManagerUniformBuffer(VulkanManagerUniformBuffer&&) = delete;

	VulkanManagerUniformBuffer& operator= (const VulkanManagerUniformBuffer&) = delete;
	VulkanManagerUniformBuffer& operator= (VulkanManagerUniformBuffer&&) = delete;

	void UpdateSwapchainUniformBuffers(const SwapchainId& swapchainId, size_t imageIndex) override;

	[[nodiscard]] DescriptorSetId CreateUniformDescriptorSet(
		const GraphicsWindowId& windowId,
		const ShaderProgramId& shaderProgramId,
		const uint32_t descriptorSetIndex) override;

	[[nodiscard]] UniformBufferId CreateUniformBuffer(
		const GraphicsWindowId& windowId,
		const description::UniformBufferType type,
		const uint64_t size) override;

	[[nodiscard]] bool CanBeUsedUniformBufferWithFollowingParameters(
		const UniformBufferId& uniformBufferId,
		const GraphicsWindowId& windowId) const override;

	[[nodiscard]] bool CanBeUsedDescriptorSetWithFollowingParameters(
		const DescriptorSetId& descriptorSetId,
		const GraphicsWindowId& windowId,
		const ShaderProgramId& shaderProgramId) const override;

	std::vector<std::string> WriteToDescriptorSet(
		const DescriptorSetId& descriptorSetId,
		const std::vector<std::pair<std::string,
		UniformBufferId>>&uniformBuffers,
		const std::vector<std::pair<std::string,
		std::vector<std::vector<TextureId>>>>&textures) override;

	void WriteToUniformBuffer(
		const UniformBufferId& uniformBufferId,
		const void* data,
		const uint64_t size,
		const uint64_t offset) override;

	void DeleteUniformDescriptorSet(const DescriptorSetId& descriptorSetId) override;

	void DeleteUniformBuffer(const UniformBufferId& uniformBufferId) override;

	[[nodiscard]] const std::vector<std::shared_ptr<DataUniformBuffer>>& GetUniformBuffer(const UniformBufferId& id) const override;

	[[nodiscard]] const std::vector<VkDescriptorSet>& GetUniformDescriptorSet(const DescriptorSetId& id) const override;

	[[nodiscard]] std::shared_ptr<DataDescriptorPool> GetDescriptorPoolBySwapchain(
		const SwapchainId& swapchainId) const override;

private:
	VulkanManagerUniformBuffer();

	[[nodiscard]] DescriptorSetId createUniformDescriptorSet(
		const ShaderProgramId& shaderProgramId,
		const SwapchainId& swapchainId,
		const LogicalDeviceId& logicalDeviceId,
		const uint32_t descriptorSetIndex);

	[[nodiscard]] UniformBufferId createUniformBuffer(
		const uint64_t size,
		const SwapchainId& swapchainId,
		const VkBufferUsageFlags usage,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId);

	std::shared_ptr<DataDescriptorPool> createDescriptorPool(
		const SwapchainId& swapchainId,
		const LogicalDeviceId& logicalDeviceId);

	std::optional<uint32_t> findUniformBufferBinding(
		const std::string& name,
		const std::vector<DataShaderModuleReflection>& dataShaderModuleReflections);

	std::optional<uint32_t> findTextureBinding(
		const std::string& name,
		const std::vector<DataShaderModuleReflection>& dataShaderModuleReflections);

	std::optional<std::vector<DataShaderModuleReflection::DescriptorSetLayoutBinding>> findDescriptorSetLayoutBindings(
		const uint32_t descriptorSetIndex,
		const std::vector<DataShaderModuleReflection>& dataShaderModuleReflections);

	bool canBeUsedUniformBufferWithFollowingParameters(
		const UniformBufferId& uniformBufferId,
		const render::LogicalDeviceId& logicalDeviceId) const;

	bool canBeUsedDescriptorSetWithFollowingParameters(
		const DescriptorSetId& descriptorSetId,
		const LogicalDeviceId& logicalDeviceId,
		const ShaderProgramId& shaderProgramId) const;

	size_t next_buffer_id_ = 0;
	size_t next_descriptor_set_id_ = 0;

	Vector<std::vector<std::shared_ptr<DataUniformBuffer>>, UniformBufferId> uniform_buffers_;
	Vector<void*, UniformBufferId> uniform_buffers_data_;
	std::map<RenderId, std::vector<std::set<UniformBufferId>>> uniform_buffer_ids_to_update_by_swapchain_id_;
	Vector<DetailUniformBuffer, UniformBufferId> detail_uniform_buffer_;

	std::vector<std::shared_ptr<DataDeviceMemory>> device_memory_;

	std::map<RenderId, std::shared_ptr<DataDescriptorPool>> descriptor_pools_by_swapchainId;

	Vector<std::vector<VkDescriptorSet>, DescriptorSetId> descriptor_sets_;
	Vector<DetailDescriptorSet, DescriptorSetId> detail_descriptor_sets_;

	std::shared_ptr<DataDevice> device_;
};

}
