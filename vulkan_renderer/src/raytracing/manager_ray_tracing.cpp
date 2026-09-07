#include "manager_ray_tracing.h"

#include "extension_functions.h"
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

	const SwapchainId swapchainId = ManagerWindow::Get()->GetSwapchainId(description.window_id);
	const LogicalDeviceId logicalDeviceId = ManagerWindow::Get()->GetLogicalDeviceId(description.window_id);
	const auto device = ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId);

	const uint32_t numImages = ManagerSwapchain::Get()->GetNumImages(swapchainId);

	if (description.camera_buffers.size() != numImages)
	{
		LOGEXC(std::runtime_error, "[ManagerRayTracing::CreatePass] expected one camera buffer per swapchain image, got:", description.camera_buffers.size());
	}

	DetailRayTracingPass pass{};
	pass.swapchain_id = swapchainId;
	pass.logical_device_id = logicalDeviceId;
	pass.extent = ManagerSwapchain::Get()->GetExtent(swapchainId);
	pass.top_level = description.top_level;

	auto managerShaderProgram = VulkanManagerShaderProgram::Get();

	// The layout comes from shader reflection, so the bindings below and the ones declared
	// in the shaders can never drift apart without the shaders failing to compile first.
	pass.descriptor_set_layout = managerShaderProgram->GetDescriptorSetLayoutData(description.shader_program_id, 0);
	pass.pipeline_layout = CreatorPipelineLayout::CreatePipelineLayout(device, { pass.descriptor_set_layout });

	const std::vector<VkPipelineShaderStageCreateInfo> stages =
		managerShaderProgram->CreatePipelineShaderStageCreateInfo(description.shader_program_id);

	pass.pipeline = CreatorRayTracingPipeline::CreatePipeline(
		device,
		pass.pipeline_layout,
		stages,
		description.max_recursion_depth);

	BuildContext context{};
	context.device = device;
	context.physical_device = ManagerDevice::Get()->GetPhysicalDevice(ManagerWindow::Get()->GetPhysicalDeviceId(description.window_id));
	context.command_pool = ManagerDevice::Get()->GetCommandPool(logicalDeviceId)->command_pool;
	context.queue = ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId);

	pass.shader_binding_table = CreatorRayTracingPipeline::CreateShaderBindingTable(context, pass.pipeline, stages);

	// One descriptor of every kind per image, which is what the fixed binding contract needs.
	const uint32_t descriptorsPerSet = 6;
	pass.descriptor_pool = CreatorDescriptorPool::CreateDescriptorPool(
		descriptorsPerSet,
		descriptorsPerSet,
		numImages,
		device);

	pass.descriptor_sets = CreatorDescriptorSet::CreateDescriptorSets(
		device,
		numImages,
		pass.descriptor_pool,
		pass.descriptor_set_layout);

	writeDescriptorSets(pass, description);

	passes_[description.window_id] = std::move(pass);

	LOG(Loglvl::info, "[ManagerRayTracing::CreatePass] ray tracing pass created for window:", description.window_id.GetId());
}

void ManagerRayTracing::writeDescriptorSets(
	DetailRayTracingPass& pass,
	const RayTracingPassDescription& description)
{
	const auto device = ManagerDevice::Get()->GetLogicalDevice(pass.logical_device_id);
	const auto& imageViews = ManagerSwapchain::Get()->GetImageViewData(pass.swapchain_id);

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
		cameraInfo.buffer = description.camera_buffers[i]->buffer;
		cameraInfo.range = VK_WHOLE_SIZE;

		VkDescriptorBufferInfo vertexInfo{};
		vertexInfo.buffer = description.vertex_buffer->buffer;
		vertexInfo.range = VK_WHOLE_SIZE;

		VkDescriptorBufferInfo indexInfo{};
		indexInfo.buffer = description.index_buffer->buffer;
		indexInfo.range = VK_WHOLE_SIZE;

		VkDescriptorBufferInfo instanceInfo{};
		instanceInfo.buffer = description.instance_buffer->buffer;
		instanceInfo.range = VK_WHOLE_SIZE;

		std::vector<VkWriteDescriptorSet> writes(6, VkWriteDescriptorSet{});

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

		initWrite(0, RayTracingBinding::AccelerationStructure, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
		writes[0].pNext = &accelerationStructureWrite;

		initWrite(1, RayTracingBinding::OutputImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		writes[1].pImageInfo = &imageInfo;

		initWrite(2, RayTracingBinding::Camera, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		writes[2].pBufferInfo = &cameraInfo;

		initWrite(3, RayTracingBinding::Vertices, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		writes[3].pBufferInfo = &vertexInfo;

		initWrite(4, RayTracingBinding::Indices, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		writes[4].pBufferInfo = &indexInfo;

		initWrite(5, RayTracingBinding::Instances, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		writes[5].pBufferInfo = &instanceInfo;

		vkUpdateDescriptorSets(
			device->device,
			static_cast<uint32_t>(writes.size()),
			writes.data(),
			0,
			nullptr);
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
