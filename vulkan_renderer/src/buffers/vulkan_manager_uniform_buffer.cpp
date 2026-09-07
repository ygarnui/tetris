#include "vulkan_manager_uniform_buffer.h"

#include "creator_vertex_buffer.h"
#include "creator_index_buffer.h"
#include "creator_uniform_buffer.h"
#include "creator_descriptor_pool.h"
#include "creator_descriptor_set_layout.h"
#include "creator_descriptor_set.h"
#include "creator_buffer.h"
#include "../src/converter_description.h"

#include <logger_instance.h>
#include <guard_next_id.h>

#include <chrono>
#include <iostream>
#include <optional>
#include <stdlib.h>

namespace render
{
	std::shared_ptr<VulkanManagerUniformBuffer>& VulkanManagerUniformBuffer::Get()
	{
		static std::shared_ptr<VulkanManagerUniformBuffer> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<VulkanManagerUniformBuffer>(new VulkanManagerUniformBuffer());
		}
		return manager;
	}

	VulkanManagerUniformBuffer::~VulkanManagerUniformBuffer()
	{
		LOG(Loglvl::debug, "[VulkanManagerUniformBuffer::~VulkanManagerUniformBuffer]");
	}

	void VulkanManagerUniformBuffer::UpdateSwapchainUniformBuffers(const SwapchainId& swapchainId, size_t imageIndex)
	{
		auto uniformBufferIds = uniform_buffer_ids_to_update_by_swapchain_id_.find(swapchainId);
		if (uniformBufferIds == uniform_buffer_ids_to_update_by_swapchain_id_.end())
		{
			return;
		}
		for (const auto& uniformBufferId : uniformBufferIds->second[imageIndex])
		{
			memcpy(
				uniform_buffers_[uniformBufferId][imageIndex]->buf_data->mapped_data,
				uniform_buffers_data_[uniformBufferId],
				uniform_buffers_[uniformBufferId][imageIndex]->buf_data->size);
		}

		uniform_buffer_ids_to_update_by_swapchain_id_[swapchainId][imageIndex].clear();
	}

	DescriptorSetId VulkanManagerUniformBuffer::CreateUniformDescriptorSet(
		const GraphicsWindowId& windowId,
		const ShaderProgramId& shaderProgramId,
		const uint32_t descriptorSetIndex)
	{
		return createUniformDescriptorSet(
			shaderProgramId,
			ManagerWindow::Get()->GetSwapchainId(windowId),
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			descriptorSetIndex);
	}

	UniformBufferId VulkanManagerUniformBuffer::CreateUniformBuffer(const GraphicsWindowId& windowId,
		const description::UniformBufferType type, const uint64_t size)
	{
		return createUniformBuffer(
			size,
			ManagerWindow::Get()->GetSwapchainId(windowId),
			ConverterDescription::ConvertUniformBufferType(type),
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerWindow::Get()->GetPhysicalDeviceId(windowId));
	}

	bool VulkanManagerUniformBuffer::CanBeUsedUniformBufferWithFollowingParameters(
		const UniformBufferId& uniformBufferId,
		const GraphicsWindowId& windowId) const
	{
		return canBeUsedUniformBufferWithFollowingParameters(
			uniformBufferId,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId));
	}

	bool VulkanManagerUniformBuffer::CanBeUsedDescriptorSetWithFollowingParameters(
		const DescriptorSetId& descriptorSetId,
		const GraphicsWindowId& windowId,
		const ShaderProgramId& shaderProgramId) const
	{
		return canBeUsedDescriptorSetWithFollowingParameters(
			descriptorSetId,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			shaderProgramId);
	}

	std::vector<std::string> VulkanManagerUniformBuffer::WriteToDescriptorSet(
		const DescriptorSetId& descriptorSetId,
		const std::vector<std::pair<std::string, UniformBufferId>>& uniformBuffers,
		const std::vector<std::pair<std::string, std::vector<std::vector<TextureId>>>>& textures)
	{
		if (uniformBuffers.empty() && textures.empty())
		{
			return {};
		}

		std::vector<std::string> invalidUniforms;

		const auto logicalDeviceId = detail_descriptor_sets_[descriptorSetId].logical_device_id;

		const auto& detailShaderProgram = VulkanManagerShaderProgram::Get()->GetDetailShaderProgram(detail_descriptor_sets_[descriptorSetId].shader_program_id);

		const size_t numUniformBuffers = uniformBuffers.size();
		std::vector<uint32_t> uniformBufferBindings;
		uniformBufferBindings.reserve(numUniformBuffers);
		std::vector<size_t> validUniformBufferIndices;
		validUniformBufferIndices.reserve(numUniformBuffers);
		for (size_t uniformBufferIndex = 0; uniformBufferIndex < numUniformBuffers; uniformBufferIndex++)
		{
			const std::optional<uint32_t> uniformBufferBinding = findUniformBufferBinding(
				uniformBuffers[uniformBufferIndex].first,
				detailShaderProgram.data_shader_module_reflections);

			if (uniformBufferBinding)
			{
				uniformBufferBindings.emplace_back(uniformBufferBinding.value());
				validUniformBufferIndices.emplace_back(uniformBufferIndex);
			}
			else
			{
				invalidUniforms.emplace_back(uniformBuffers[uniformBufferIndex].first);
			}
		}

		const size_t numTextures = textures.size();
		std::vector<uint32_t> textureBindings;
		textureBindings.reserve(numTextures);
		for (size_t textureIndex = 0; textureIndex < numTextures; textureIndex++)
		{
			const std::optional<uint32_t> textureBinding = findTextureBinding(
				textures[textureIndex].first,
				detailShaderProgram.data_shader_module_reflections);

			if (textureBinding)
			{
				textureBindings.emplace_back(textureBinding.value());
			}
			else
			{
				invalidUniforms.emplace_back(textures[textureIndex].first);
			}
		}

		const auto descriptorSetLayoutBindings = findDescriptorSetLayoutBindings(
			detail_descriptor_sets_[descriptorSetId].descriptor_set_index,
			detailShaderProgram.data_shader_module_reflections);

		if (!descriptorSetLayoutBindings)
		{
			LOGEXC(std::runtime_error, "Shader Reflection doesn't contain descriptor set index " + std::to_string(detail_descriptor_sets_[descriptorSetId].descriptor_set_index));
		}

		size_t numBufferLeveling = descriptor_sets_[descriptorSetId].size();
		for (size_t descriptorSetIndexInSwapchain = 0; descriptorSetIndexInSwapchain < numBufferLeveling; descriptorSetIndexInSwapchain++)
		{
			std::vector<std::shared_ptr<DataUniformBuffer>> buffersBySwapchain;
			buffersBySwapchain.reserve(numUniformBuffers);

			for (const uint64_t uniformBufferIndex : validUniformBufferIndices)
			{
				auto uniformBufferId = uniformBuffers[uniformBufferIndex].second;

				if (logicalDeviceId != detail_uniform_buffer_[uniformBufferId].logical_device_id)
				{
					LOGEXC(std::runtime_error, "[VulkanManagerUniformBuffer::WriteToDescriptorSet] invalid set uniform buffer in uniform set");
				}

				buffersBySwapchain.emplace_back(uniform_buffers_[uniformBufferId][descriptorSetIndexInSwapchain]);
			}

			std::vector<std::vector<std::shared_ptr<DataImageView>>> imageViews;
			imageViews.reserve(numTextures);
			std::vector<std::vector<std::shared_ptr<DataSampler>>> samplers;
			samplers.reserve(numTextures);
			std::vector<std::vector<VkImageLayout>> textureLayout;
			textureLayout.reserve(numTextures);

			for (size_t textureIndex = 0; textureIndex < numTextures; textureIndex++)
			{
				std::vector<std::shared_ptr<DataImageView>>& imageViewsPerTexture = imageViews.emplace_back();
				std::vector<std::shared_ptr<DataSampler>>& samplersPerTexture = samplers.emplace_back();
				std::vector<VkImageLayout>& layoutPerTexture = textureLayout.emplace_back();

				if (textures[textureIndex].second.size() == numBufferLeveling)
				{
					const auto& texture = textures[textureIndex].second[descriptorSetIndexInSwapchain];
					if (texture.empty())
					{
						LOGEXC(std::runtime_error, "There are no textures, failed to write textures to the descriptor set:", (size_t)descriptorSetId.GetId());
					}

					if (texture.size() == 1)
					{
						layoutPerTexture.emplace_back(VulkanManagerTextures::Get()->GetImageLayout(texture.back()));
						imageViewsPerTexture.emplace_back(VulkanManagerTextures::Get()->GetImageViewData(texture.back()));
						samplersPerTexture.emplace_back(VulkanManagerTextures::Get()->GetSamplerData(texture.back()));
					}
					else if (texture.size() == descriptor_sets_[descriptorSetId].size())
					{
						layoutPerTexture.emplace_back(VulkanManagerTextures::Get()->GetImageLayout(texture[descriptorSetIndexInSwapchain]));
						imageViewsPerTexture.emplace_back(VulkanManagerTextures::Get()->GetImageViewData(texture[descriptorSetIndexInSwapchain]));
						samplersPerTexture.emplace_back(VulkanManagerTextures::Get()->GetSamplerData(texture[descriptorSetIndexInSwapchain]));
					}
					else
					{
						LOGEXC(std::runtime_error, "Number of textures is different from the swapchain image count, failed to write textures to the descriptor set:", (size_t)descriptorSetId.GetId());
					}
				}
				else
				{
					for (const auto& texture : textures[textureIndex].second)
					{
						if (texture.empty())
						{
							LOGEXC(std::runtime_error, "There are no textures, failed to write textures to the descriptor set:", (size_t)descriptorSetId.GetId());
						}

						if (texture.size() == 1)
						{
							layoutPerTexture.emplace_back(VulkanManagerTextures::Get()->GetImageLayout(texture.back()));
							imageViewsPerTexture.emplace_back(VulkanManagerTextures::Get()->GetImageViewData(texture.back()));
							samplersPerTexture.emplace_back(VulkanManagerTextures::Get()->GetSamplerData(texture.back()));
						}
						else if (texture.size() == descriptor_sets_[descriptorSetId].size())
						{
							layoutPerTexture.emplace_back(VulkanManagerTextures::Get()->GetImageLayout(texture[descriptorSetIndexInSwapchain]));
							imageViewsPerTexture.emplace_back(VulkanManagerTextures::Get()->GetImageViewData(texture[descriptorSetIndexInSwapchain]));
							samplersPerTexture.emplace_back(VulkanManagerTextures::Get()->GetSamplerData(texture[descriptorSetIndexInSwapchain]));
						}
						else
						{
							LOGEXC(std::runtime_error, "Number of textures is different from the swapchain image count, failed to write textures to the descriptor set:", (size_t)descriptorSetId.GetId());
						}
					}
				}
			}

			CreatorDescriptorSet::WriteToDescriptorSet(
				descriptor_sets_[descriptorSetId][descriptorSetIndexInSwapchain],
				uniformBufferBindings,
				buffersBySwapchain,
				textureBindings,
				textureLayout,
				imageViews,
				samplers,
				descriptorSetLayoutBindings.value_or(std::vector<DataShaderModuleReflection::DescriptorSetLayoutBinding>()),
				ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId));
		}

		return invalidUniforms;
	}

	void VulkanManagerUniformBuffer::WriteToUniformBuffer(
		const UniformBufferId& uniformBufferId,
		const void* data,
		const uint64_t size,
		const uint64_t offset)
	{
		void* uniformBufferData = uniform_buffers_data_[uniformBufferId];
		if (offset + size > uniform_buffers_[uniformBufferId][0]->buf_data->size)
		{
			LOGEXC(std::runtime_error, "[VulkanVulkanManagerUniformBuffer::WriteToUniformBuffer] Failed to write to uniform buffer " +
				std::to_string(uniformBufferId.GetId()) + ", value is out of range!");
		}

		std::copy((char*)data, (char*)data + size, (char*)uniformBufferData + offset);

		const SwapchainId& swapchainId = detail_uniform_buffer_[uniformBufferId].swapchain_id;
		const uint32_t bufferingLevel = static_cast<uint32_t>(ManagerSwapchain::Get()->GetNumImages(swapchainId));
		auto uniformBufferIdsToUpdateBySwapchainId = uniform_buffer_ids_to_update_by_swapchain_id_.find(swapchainId);
		if (uniformBufferIdsToUpdateBySwapchainId == uniform_buffer_ids_to_update_by_swapchain_id_.end())
		{
			uniformBufferIdsToUpdateBySwapchainId = uniform_buffer_ids_to_update_by_swapchain_id_.emplace(
				swapchainId,
				std::vector<std::set<UniformBufferId>>{ bufferingLevel }).first;
		}

		for (auto& swapchainUniformBuffer : uniformBufferIdsToUpdateBySwapchainId->second)
		{
			swapchainUniformBuffer.emplace(uniformBufferId);
		}
	}

	void VulkanManagerUniformBuffer::DeleteUniformDescriptorSet(const DescriptorSetId& descriptorSetId)
	{
		if (descriptorSetId >= descriptor_sets_.size())
		{
			LOGEXC(std::runtime_error, "[VulkanManagerUniformBuffer::DeleteUniformDescriptorSet] Descriptor set id " + std::to_string(descriptorSetId.GetId()) + " to delete is out of range!");
		}

		const auto detailDescriptorSets = detail_descriptor_sets_[descriptorSetId];
		CreatorDescriptorSet::DeleteDescriptorSets(
			ManagerDevice::Get()->GetLogicalDevice(detailDescriptorSets.logical_device_id),
			GetDescriptorPoolBySwapchain(detailDescriptorSets.swapchain_id),
			descriptor_sets_[descriptorSetId]);

		for (auto& descriptorSet : descriptor_sets_[descriptorSetId])
		{
			descriptorSet = VK_NULL_HANDLE;
		}

		detail_descriptor_sets_[descriptorSetId] = {};

		next_descriptor_set_id_ = std::min(descriptorSetId.GetId(), next_descriptor_set_id_);
	}

	void VulkanManagerUniformBuffer::DeleteUniformBuffer(const UniformBufferId& uniformBufferId)
	{
		for (auto& uniformBuffer : uniform_buffers_[uniformBufferId])
		{
			uniformBuffer = nullptr;
		}

		std::free(uniform_buffers_data_[uniformBufferId]);
		detail_uniform_buffer_[uniformBufferId] = {};
		next_buffer_id_ = std::min(size_t(uniformBufferId), next_buffer_id_);
	}

	const std::vector<std::shared_ptr<DataUniformBuffer>>& VulkanManagerUniformBuffer::GetUniformBuffer(const render::UniformBufferId& id) const
	{
		return uniform_buffers_[id];
	}

	const std::vector<VkDescriptorSet>& VulkanManagerUniformBuffer::GetUniformDescriptorSet(const render::DescriptorSetId& id) const
	{
		return descriptor_sets_[id];
	}

	std::shared_ptr<DataDescriptorPool> VulkanManagerUniformBuffer::GetDescriptorPoolBySwapchain(const render::SwapchainId& swapchainId) const
	{
		auto descriptor_pool_by_swapchainId = descriptor_pools_by_swapchainId.find(swapchainId);
		if (descriptor_pool_by_swapchainId != descriptor_pools_by_swapchainId.end())
		{
			return descriptor_pool_by_swapchainId->second;
		}

		return nullptr;
	}

	VulkanManagerUniformBuffer::VulkanManagerUniformBuffer()
	{

	}

	DescriptorSetId VulkanManagerUniformBuffer::createUniformDescriptorSet(
		const ShaderProgramId& shaderProgramId,
		const SwapchainId& swapchainId,
		const LogicalDeviceId& logicalDeviceId,
		const uint32_t descriptorSetIndex)
	{
		std::shared_ptr<DataDescriptorPool> dataDescriptorPool = GetDescriptorPoolBySwapchain(swapchainId);
		const uint32_t bufferingLevel = static_cast<uint32_t>(ManagerSwapchain::Get()->GetNumImages(swapchainId));
		if (!dataDescriptorPool)
		{
			dataDescriptorPool = createDescriptorPool(swapchainId, logicalDeviceId);
		}

		const auto descriptorSetLayoutData = VulkanManagerShaderProgram::Get()->GetDescriptorSetLayoutData(shaderProgramId, descriptorSetIndex);

		auto guardResize = utils::GuardResize::MayBeResize(
			next_descriptor_set_id_,
			[&](const std::vector<VkDescriptorSet>& vec) { return vec[0] == VK_NULL_HANDLE; },
			descriptor_sets_,
			detail_descriptor_sets_);

		const DescriptorSetId descriptorSetId = GeneratorId::GenerateUniqueId<DescriptorSetId>(next_descriptor_set_id_);

		auto descriptorSets = CreatorDescriptorSet::CreateDescriptorSets(
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			bufferingLevel,
			dataDescriptorPool,
			descriptorSetLayoutData);

		descriptor_sets_[descriptorSetId] = descriptorSets;
		DetailDescriptorSet detailDescriptorSet;
		detailDescriptorSet.logical_device_id = logicalDeviceId;
		detailDescriptorSet.shader_program_id = shaderProgramId;
		detailDescriptorSet.swapchain_id = swapchainId;
		detailDescriptorSet.descriptor_set_index = descriptorSetIndex;
		detail_descriptor_sets_[descriptorSetId] = { logicalDeviceId, shaderProgramId, swapchainId, descriptorSetIndex };

		return descriptorSetId;
	}

	UniformBufferId VulkanManagerUniformBuffer::createUniformBuffer(
		const uint64_t size,
		const SwapchainId& swapchainId,
		const VkBufferUsageFlags usage,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId)
	{
		auto guardId = utils::GuardResize::MayBeResize(
			next_buffer_id_,
			[&](const std::vector<std::shared_ptr<DataUniformBuffer>>& vec) { return bool(!vec[0]); },
			uniform_buffers_,
			detail_uniform_buffer_,
			uniform_buffers_data_);

		UniformBufferId newId = GeneratorId::GenerateUniqueId<UniformBufferId>(next_buffer_id_);

		uint32_t bufferingLevel = static_cast<uint32_t>(ManagerSwapchain::Get()->GetNumImages(swapchainId));

		uniform_buffers_data_[newId] = new char[size];
		uniform_buffers_[newId] = std::vector<std::shared_ptr<DataUniformBuffer>>(bufferingLevel);
		detail_uniform_buffer_[newId] = { logicalDeviceId, swapchainId, physicalDeviceId};

		std::vector<std::shared_ptr<DataUniformBuffer>>& uniformBuffers = uniform_buffers_[newId];
		for (std::shared_ptr<DataUniformBuffer>& uniformBuffer : uniformBuffers)
		{
			uniformBuffer = CreatorUniformBuffer::CreateUniformBuffer(
				size,
				usage,
				ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
				ManagerDevice::Get()->GetPhysicalDevice(physicalDeviceId));

			// Recommended if any buffer is created on cpu access memory map it always on creation.
			// TODO: maybe pass the whole DataBuffer;
			uniformBuffer->buf_data->mapped_data = CreatorBuffer::Map(size, 0, uniformBuffer->device_memory);
		}
		
		return newId;
	}

	std::shared_ptr<DataDescriptorPool> VulkanManagerUniformBuffer::createDescriptorPool(
		const SwapchainId& swapchainId,
		const LogicalDeviceId& logicalDeviceId)
	{
		const uint32_t bufferingLevel = static_cast<uint32_t>(ManagerSwapchain::Get()->GetNumImages(swapchainId));
		descriptor_pools_by_swapchainId[swapchainId] = CreatorDescriptorPool::CreateDescriptorPool(
			1000,
			1000,
			bufferingLevel,
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId));
		
		return descriptor_pools_by_swapchainId[swapchainId];
	}

	std::optional<uint32_t> VulkanManagerUniformBuffer::findUniformBufferBinding(
		const std::string& name,
		const std::vector<DataShaderModuleReflection>& dataShaderModuleReflections)
	{
		for (const auto& dataShaderModuleReflection : dataShaderModuleReflections)
		{
			auto uniformBuffer = dataShaderModuleReflection.uniform_buffers_by_name.find(name);
			if (uniformBuffer != dataShaderModuleReflection.uniform_buffers_by_name.end())
			{
				return uniformBuffer->second.resource_location.binding;
			}
		}

		return std::nullopt;
	}

	std::optional<uint32_t> VulkanManagerUniformBuffer::findTextureBinding(
		const std::string& name,
		const std::vector<DataShaderModuleReflection>& dataShaderModuleReflections)
	{
		for (const auto& dataShaderModuleReflection : dataShaderModuleReflections)
		{
			auto uniformBuffer = dataShaderModuleReflection.sampled_images_by_name.find(name);
			if (uniformBuffer != dataShaderModuleReflection.sampled_images_by_name.end())
			{
				return uniformBuffer->second.resource_location.binding;
			}
		}

		return std::nullopt;
	}

	std::optional<std::vector<DataShaderModuleReflection::DescriptorSetLayoutBinding>> VulkanManagerUniformBuffer::findDescriptorSetLayoutBindings(
		const uint32_t descriptorSetIndex,
		const std::vector<DataShaderModuleReflection>& dataShaderModuleReflections)
	{
		std::vector<DataShaderModuleReflection::DescriptorSetLayoutBinding> vec;
		for (const auto& dataShaderModuleReflection : dataShaderModuleReflections)
		{
			auto descriptorSetLayoutBindings = dataShaderModuleReflection.bindings_by_descriptor_set.find(descriptorSetIndex);
			if (descriptorSetLayoutBindings != dataShaderModuleReflection.bindings_by_descriptor_set.end())
			{
				for (const auto& binding : descriptorSetLayoutBindings->second)
				{
					vec.push_back(binding);
				}
				
			}
		}

		if (!vec.empty())
		{
			return vec;
		}

		return std::nullopt;
	}

	bool VulkanManagerUniformBuffer::canBeUsedUniformBufferWithFollowingParameters(
		const render::UniformBufferId& uniformBufferId,
		const render::LogicalDeviceId& logicalDeviceId) const
	{
		auto& detail = detail_uniform_buffer_[uniformBufferId];
		if (detail.logical_device_id == logicalDeviceId)
		{
			return true;
		}

		return false;
	}

	bool VulkanManagerUniformBuffer::canBeUsedDescriptorSetWithFollowingParameters(
		const render::DescriptorSetId& descriptorSetId,
		const render::LogicalDeviceId& logicalDeviceId,
		const render::ShaderProgramId& shaderProgramId) const
	{
		auto& detail = detail_descriptor_sets_[descriptorSetId];
		if (detail.logical_device_id == logicalDeviceId &&
			detail.shader_program_id == shaderProgramId)
		{
			return true;
		}

		return false;
	}

}
