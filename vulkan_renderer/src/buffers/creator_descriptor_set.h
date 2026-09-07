#pragma once

#include "../struct_data.h"

#include <data_shader_module_reflection.h>

namespace render
{
	class CreatorDescriptorSet
	{
	public:
		[[nodiscard]] static std::vector<VkDescriptorSet> CreateDescriptorSets(
			std::shared_ptr<DataDevice> device,
			const uint32_t numSwapChainImages,
			std::shared_ptr<DataDescriptorPool> descriptorPool,
			std::shared_ptr<DataDescriptorSetLayout> descriptorSetLayout);

		static void DeleteDescriptorSets(
			std::shared_ptr<DataDevice> device,
			std::shared_ptr<DataDescriptorPool> descriptorPool,
			const std::vector<VkDescriptorSet>& descriptorSets);

		static void WriteToDescriptorSet(
			const VkDescriptorSet descriptorSet,
			const std::vector<uint32_t>& uniformBindings,
			const std::vector<std::shared_ptr<DataUniformBuffer>>& uniformBuffers,
			const std::vector<uint32_t>& samplerBindings,
			const std::vector<std::vector<VkImageLayout>>& textureLayout,
			const std::vector<std::vector<std::shared_ptr<DataImageView>>>& imageViews,
			const std::vector<std::vector<std::shared_ptr<DataSampler>>>& samplers,
			const std::vector<DataShaderModuleReflection::DescriptorSetLayoutBinding>& descriptorSetLayoutBindings,
			std::shared_ptr<DataDevice> device);

	private:
};
}
