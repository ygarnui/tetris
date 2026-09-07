#include "creator_descriptor_set.h"

#include "../converter_description.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{

std::vector<VkDescriptorSet> CreatorDescriptorSet::CreateDescriptorSets(
	std::shared_ptr<DataDevice> device,
	const uint32_t numSwapChainImages,
	std::shared_ptr<DataDescriptorPool> descriptorPool,
	std::shared_ptr<DataDescriptorSetLayout> descriptorSetLayout)
{
	std::vector<VkDescriptorSetLayout> layouts(numSwapChainImages, descriptorSetLayout->descriptor_set_layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool->descriptor_pool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	std::vector<VkDescriptorSet> descriptorSets(numSwapChainImages);

	const VkResult result = vkAllocateDescriptorSets(device->device, &allocInfo, descriptorSets.data());
	if (result != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorDescriptorSet::CreateDescriptorSets] failed to allocate descriptor sets!");
	}

	return descriptorSets;
}

void CreatorDescriptorSet::DeleteDescriptorSets(
	std::shared_ptr<DataDevice> device,
	std::shared_ptr<DataDescriptorPool> descriptorPool,
	const std::vector<VkDescriptorSet>& descriptorSets)
{
	vkFreeDescriptorSets(device->device, descriptorPool->descriptor_pool, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data());
}

void CreatorDescriptorSet::WriteToDescriptorSet(
	const VkDescriptorSet descriptorSet,
	const std::vector<uint32_t>& uniformBindings,
	const std::vector<std::shared_ptr<DataUniformBuffer>>& uniformBuffers,
	const std::vector<uint32_t>& samplerBindings,
	const std::vector<std::vector<VkImageLayout>>& textureLayout,
	const std::vector<std::vector<std::shared_ptr<DataImageView>>>& imageViews,
	const std::vector<std::vector<std::shared_ptr<DataSampler>>>& samplers,
	const std::vector<DataShaderModuleReflection::DescriptorSetLayoutBinding>& descriptorSetLayoutBindings,
	std::shared_ptr<DataDevice> device)
{
	const size_t numUniformBuffers = uniformBuffers.size();
	const size_t numSamplers = imageViews.size();
	const size_t numWrites = numUniformBuffers + numSamplers;
	std::vector<VkDescriptorBufferInfo> bufferInfos(numUniformBuffers);
	std::vector<std::vector<VkDescriptorImageInfo>> imageInfos(numSamplers);
	std::vector<VkWriteDescriptorSet> writes;
	writes.reserve(numWrites);

	for (size_t uniformBufferIndex = 0; uniformBufferIndex < numUniformBuffers; uniformBufferIndex++)
	{
		VkDescriptorBufferInfo& bufferInfo = bufferInfos[uniformBufferIndex];
		VkWriteDescriptorSet write{};

		bufferInfo.buffer = uniformBuffers[uniformBufferIndex]->buf_data->buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = uniformBuffers[uniformBufferIndex]->buf_data->size;

		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = descriptorSet;
		write.dstBinding = uniformBindings[uniformBufferIndex];
		write.dstArrayElement = 0; 
		write.descriptorType = ConverterDescription::ConvertUniformTypeToDescriptorType(descriptorSetLayoutBindings[uniformBindings[uniformBufferIndex]].type);
		write.descriptorCount = 1;
		write.pBufferInfo = &bufferInfo;
		write.pImageInfo = nullptr;
		write.pTexelBufferView = nullptr;

		writes.push_back(write);
	}

	for (size_t samplerIndex = 0; samplerIndex < numSamplers; samplerIndex++)
	{
		std::vector<VkDescriptorImageInfo>& imageInfosPerBinding = imageInfos[samplerIndex];

		for (size_t samplerIndexPerBinding = 0; samplerIndexPerBinding < samplers[samplerIndex].size(); samplerIndexPerBinding++)
		{
			VkDescriptorImageInfo& imageInfo = imageInfosPerBinding.emplace_back();

			imageInfo.imageView = imageViews[samplerIndex][samplerIndexPerBinding]->image_view;
			imageInfo.imageLayout = textureLayout[samplerIndex][samplerIndexPerBinding]; 
			if (samplers[samplerIndex][samplerIndexPerBinding])
				imageInfo.sampler = samplers[samplerIndex][samplerIndexPerBinding]->sampler;
		}

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = descriptorSet; 
		write.dstBinding = samplerBindings[samplerIndex];
		write.dstArrayElement = 0;
		//TO DO
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = static_cast<uint32_t>(samplers[samplerIndex].size());
		write.pBufferInfo = nullptr;
		write.pImageInfo = imageInfosPerBinding.data();
		write.pTexelBufferView = nullptr;

		writes.push_back(write);
	}

	vkUpdateDescriptorSets(device->device, static_cast<uint32_t>(writes.size()), writes.data(), static_cast<uint32_t>(0), nullptr);
}

}
