#include "manager_ray_tracing.h"

#include "extension_functions.h"
#include "../buffers/creator_buffer.h"
#include "../buffers/creator_descriptor_pool.h"
#include "../buffers/creator_descriptor_set.h"
#include "../manager_device.h"
#include "../manager_swapchain.h"
#include "../manager_window.h"
#include "../pipeline/creator_pipeline_layout.h"
#include "../shader/vulkan_manager_shader_program.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{

namespace
{
	/*!
	\brief Number of descriptors a single ray tracing set holds, as fixed by RayTracingBinding.
	*/
	constexpr uint32_t descriptors_per_set = 5;

	/*!
	\brief Mirrors RT_MAX_TEXTURES in globals/raytracing.h - see the comment there.
	*/
	constexpr uint32_t max_textures = 16;

	BuildContext makeBuildContext(
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId)
	{
		BuildContext context{};
		context.device = ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId);
		context.physical_device = ManagerDevice::Get()->GetPhysicalDevice(physicalDeviceId);
		context.command_pool = ManagerDevice::Get()->GetCommandPool(logicalDeviceId)->command_pool;
		context.queue = ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId);

		return context;
	}
}

std::shared_ptr<ManagerRayTracing>& ManagerRayTracing::Get()
{
	static std::shared_ptr<ManagerRayTracing> manager;
	if (!manager || manager->NeedReinit())
	{
		manager = std::shared_ptr<ManagerRayTracing>(new ManagerRayTracing());
	}
	return manager;
}

ManagerRayTracing::~ManagerRayTracing()
{
	LOG(Loglvl::debug, "[ManagerRayTracing::~ManagerRayTracing]");
}

void ManagerRayTracing::CreatePass(const RayTracingPassDescription& description)
{
	if (passes_.contains(description.window_id))
	{
		LOGEXC(std::runtime_error, "[ManagerRayTracing::CreatePass] the window already has a ray tracing pass");
	}

	if (description.uniform_buffer_size == 0)
	{
		LOGEXC(std::invalid_argument, "[ManagerRayTracing::CreatePass] uniform_buffer_size is 0");
	}

	if (description.textures.empty())
	{
		LOGEXC(std::invalid_argument, "[ManagerRayTracing::CreatePass] textures must have at least one entry");
	}

	DetailRayTracingPass pass{};
	pass.description = description;
	pass.swapchain_id = ManagerWindow::Get()->GetSwapchainId(description.window_id);
	pass.logical_device_id = ManagerWindow::Get()->GetLogicalDeviceId(description.window_id);
	pass.physical_device_id = ManagerWindow::Get()->GetPhysicalDeviceId(description.window_id);
	pass.extent = ManagerSwapchain::Get()->GetExtent(pass.swapchain_id);
	pass.top_level = description.top_level;

	const auto device = ManagerDevice::Get()->GetLogicalDevice(pass.logical_device_id);

	auto managerShaderProgram = VulkanManagerShaderProgram::Get();

	// The layout comes from shader reflection, so the bindings written below and the ones
	// declared in the shaders cannot drift apart without the shaders failing to compile first.
	pass.descriptor_set_layout = managerShaderProgram->GetDescriptorSetLayoutData(description.shader_program_id, 0);
	pass.pipeline_layout = CreatorPipelineLayout::CreatePipelineLayout(device, { pass.descriptor_set_layout });

	const std::vector<VkPipelineShaderStageCreateInfo> stages =
		managerShaderProgram->CreatePipelineShaderStageCreateInfo(description.shader_program_id);

	pass.pipeline = CreatorRayTracingPipeline::CreatePipeline(
		device,
		pass.pipeline_layout,
		stages,
		description.max_recursion_depth);

	const BuildContext context = makeBuildContext(pass.logical_device_id, pass.physical_device_id);

	pass.shader_binding_table = CreatorRayTracingPipeline::CreateShaderBindingTable(context, pass.pipeline, stages);

	createDescriptorResources(pass, ManagerSwapchain::Get()->GetNumImages(pass.swapchain_id));
	writeDescriptorSets(pass);

	passes_[description.window_id] = std::move(pass);

	LOG(Loglvl::info, "[ManagerRayTracing::CreatePass] ray tracing pass created for window:", description.window_id.GetId());
}

void ManagerRayTracing::createDescriptorResources(DetailRayTracingPass& pass, const uint32_t numImages)
{
	const auto device = ManagerDevice::Get()->GetLogicalDevice(pass.logical_device_id);
	const BuildContext context = makeBuildContext(pass.logical_device_id, pass.physical_device_id);

	// Dropping the pool frees the sets allocated from it, so nothing else has to be released.
	pass.descriptor_sets.clear();
	pass.descriptor_pool = CreatorDescriptorPool::CreateDescriptorPool(
		descriptors_per_set,
		max_textures,
		numImages,
		device);

	pass.descriptor_sets = CreatorDescriptorSet::CreateDescriptorSets(
		device,
		numImages,
		pass.descriptor_pool,
		pass.descriptor_set_layout);

	pass.uniform_buffers.resize(numImages);
	for (uint32_t i = 0; i < numImages; ++i)
	{
		pass.uniform_buffers[i] = CreatorAccelerationStructure::CreateDeviceAddressBuffer(
			context,
			nullptr,
			pass.description.uniform_buffer_size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	}
}

void ManagerRayTracing::writeDescriptorSets(DetailRayTracingPass& pass)
{
	const auto device = ManagerDevice::Get()->GetLogicalDevice(pass.logical_device_id);
	const auto& imageViews = ManagerSwapchain::Get()->GetImageViewData(pass.swapchain_id);

	// Every element of the shader's texture array must be a valid descriptor; slots beyond
	// what the caller supplied repeat its last entry. Copy it out before resize() rather than
	// passing paddedTextures.back() straight in, since that reference would dangle the moment
	// the resize reallocates the very vector it came from.
	std::vector<VkDescriptorImageInfo> paddedTextures = pass.description.textures;
	const VkDescriptorImageInfo lastTexture = paddedTextures.back();
	paddedTextures.resize(max_textures, lastTexture);

	for (size_t i = 0; i < pass.descriptor_sets.size(); ++i)
	{
		VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureWrite{};
		accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		accelerationStructureWrite.accelerationStructureCount = 1;
		accelerationStructureWrite.pAccelerationStructures = &pass.top_level->acceleration_structure;

		// The output image is the swapchain image of this index: the ray generation shader
		// writes the final pixels directly, there is no intermediate render target.
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = imageViews[i]->image_view;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkDescriptorBufferInfo cameraInfo{};
		cameraInfo.buffer = pass.uniform_buffers[i].buffer->buffer;
		cameraInfo.range = VK_WHOLE_SIZE;

		VkDescriptorBufferInfo instanceInfo{};
		instanceInfo.buffer = pass.description.instance_buffer->buffer;
		instanceInfo.range = VK_WHOLE_SIZE;

		std::vector<VkWriteDescriptorSet> writes(descriptors_per_set, VkWriteDescriptorSet{});

		auto initWrite = [&](
			const size_t index,
			const RayTracingBinding binding,
			const VkDescriptorType type)
		{
			writes[index].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[index].dstSet = pass.descriptor_sets[i];
			writes[index].dstBinding = static_cast<uint32_t>(binding);
			writes[index].descriptorCount = 1;
			writes[index].descriptorType = type;
		};

		uint32_t pos = 0;
		initWrite(pos, RayTracingBinding::AccelerationStructure, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
		writes[pos++].pNext = &accelerationStructureWrite;

		initWrite(pos, RayTracingBinding::OutputImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		writes[pos++].pImageInfo = &imageInfo;

		initWrite(pos, RayTracingBinding::Camera, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		writes[pos++].pBufferInfo = &cameraInfo;

		initWrite(pos, RayTracingBinding::Instances, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		writes[pos++].pBufferInfo = &instanceInfo;

		initWrite(pos, RayTracingBinding::Textures, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		writes[pos].descriptorCount = static_cast<uint32_t>(paddedTextures.size());
		writes[pos++].pImageInfo = paddedTextures.data();

		vkUpdateDescriptorSets(
			device->device,
			static_cast<uint32_t>(writes.size()),
			writes.data(),
			0,
			nullptr);
	}
}

void ManagerRayTracing::ApplyResize(const SwapchainId& swapchainId)
{
	for (auto& [windowId, pass] : passes_)
	{
		if (pass.swapchain_id != swapchainId)
		{
			continue;
		}

		pass.extent = ManagerSwapchain::Get()->GetExtent(swapchainId);

		const uint32_t numImages = ManagerSwapchain::Get()->GetNumImages(swapchainId);

		// A different image count means a different number of descriptor sets and uniform
		// buffers, so those are rebuilt; otherwise the existing sets are just rewritten to
		// point at the new image views.
		if (numImages != pass.descriptor_sets.size())
		{
			createDescriptorResources(pass, numImages);
		}

		writeDescriptorSets(pass);

		LOG(Loglvl::info, "[ManagerRayTracing::ApplyResize] pass rebound, extent:", pass.extent.width, "x", pass.extent.height);
	}
}

void ManagerRayTracing::SetUniform(
	const GraphicsWindowId& windowId,
	const void* data,
	const uint64_t size)
{
	const auto found = passes_.find(windowId);
	if (found == passes_.end())
	{
		LOGEXC(std::runtime_error, "[ManagerRayTracing::SetUniform] the window has no ray tracing pass");
	}

	DetailRayTracingPass& pass = found->second;

	if (size > pass.description.uniform_buffer_size)
	{
		LOGEXC(std::invalid_argument, "[ManagerRayTracing::SetUniform] data does not fit the uniform buffer, size:", size);
	}

	const auto* bytes = static_cast<const uint8_t*>(data);
	pass.staged_uniform.assign(bytes, bytes + size);
}

void ManagerRayTracing::UploadUniform(const SwapchainId& swapchainId, const uint32_t imageIndex)
{
	for (auto& [windowId, pass] : passes_)
	{
		if (pass.swapchain_id != swapchainId || pass.staged_uniform.empty())
		{
			continue;
		}

		if (imageIndex >= pass.uniform_buffers.size())
		{
			LOGEXC(std::out_of_range, "[ManagerRayTracing::UploadUniform] image index out of range:", imageIndex);
		}

		CreatorBuffer::Write(
			pass.staged_uniform.data(),
			pass.staged_uniform.size(),
			pass.uniform_buffers[imageIndex].device_memory);
	}
}

bool ManagerRayTracing::HasPass(const GraphicsWindowId& windowId) const
{
	return passes_.contains(windowId);
}

const DetailRayTracingPass& ManagerRayTracing::GetPass(const GraphicsWindowId& windowId) const
{
	const auto pass = passes_.find(windowId);
	if (pass == passes_.end())
	{
		LOGEXC(std::runtime_error, "[ManagerRayTracing::GetPass] the window has no ray tracing pass");
	}
	return pass->second;
}

void ManagerRayTracing::UpdateTopLevel(
	const GraphicsWindowId& windowId,
	std::shared_ptr<DataAccelerationStructure> topLevel)
{
	const auto found = passes_.find(windowId);
	if (found == passes_.end())
	{
		LOGEXC(std::runtime_error, "[ManagerRayTracing::UpdateTopLevel] the window has no ray tracing pass");
	}

	DetailRayTracingPass& pass = found->second;
	pass.top_level = topLevel;
	pass.description.top_level = topLevel;

	const auto device = ManagerDevice::Get()->GetLogicalDevice(pass.logical_device_id);

	for (const VkDescriptorSet descriptorSet : pass.descriptor_sets)
	{
		VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureWrite{};
		accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		accelerationStructureWrite.accelerationStructureCount = 1;
		accelerationStructureWrite.pAccelerationStructures = &pass.top_level->acceleration_structure;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = &accelerationStructureWrite;
		write.dstSet = descriptorSet;
		write.dstBinding = static_cast<uint32_t>(RayTracingBinding::AccelerationStructure);
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		vkUpdateDescriptorSets(device->device, 1, &write, 0, nullptr);
	}
}

void ManagerRayTracing::DeletePass(const GraphicsWindowId& windowId)
{
	passes_.erase(windowId);
}

}
