#include "../include/vulkan_render_base.h"

#include "creator_instance.h"
#include "validation_layer.h"
#include "creator_debug_utils_messenger.h"
#include "converter_description.h"
#include "pipeline/vulkan_manager_drawcall.h"
#include "vulkan_manager_assets.h"

#include "shader/compiler_shader_module.h"
#include "shader/reader_shader.h"
#include "raytracing/extension_functions.h"
#include "raytracing/manager_ray_tracing.h"

#include <logger_instance.h>
#include <image_loader.h>

#include <algorithm>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <iostream>

#define VK_CHECK(result)                                                                                     \
	if (result != VK_SUCCESS)                                                                                \
	{                                                                                                        \
		std::cout << "Vulkan error at " << __FILE__ << ":" << __LINE__ << " - code " << result << std::endl; \
	}

namespace render
{
	namespace
	{
		/*!
		\brief Device extensions every window needs.

		The ray tracing pipeline and the acceleration structure extensions are always requested:
		the renderer has no raster only fallback, so a device without them is simply not suitable.
		VK_KHR_deferred_host_operations is a hard dependency of the acceleration structure extension,
		even though the builds here are all done on the device.
		*/
		std::vector<const char*> requiredDeviceExtensions()
		{
			return {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME,
				VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
				VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
				VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			};
		}
	}

	// Основная функция создания всех ресурсов для culling
	CullingResources VulkanRenderBase::createCullingResources(
		GraphicsWindowId idWindow,
		uint32_t maxPatches,
		VertexBufferId vertexBufferId,
		UniformBufferId uniformBufferId
	)
	{
		VkDevice device = ManagerDevice::Get()->GetLogicalDevice(ManagerWindow::Get()->GetLogicalDeviceId(idWindow))->device;
		VkPhysicalDevice physicalDevice = ManagerDevice::Get()->GetPhysicalDevice(ManagerWindow::Get()->GetPhysicalDeviceId(idWindow));
		CullingResources resources = {};
		resources.maxPatches = maxPatches;

		// 1. Создание дескриптор сет layout
		std::vector<VkDescriptorSetLayoutBinding> bindings = {
			// Вершинный буфер (binding 0)
			{ 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr },
			// Indirect буфер (binding 1)
			{ 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr },
			// Буфер счетчик (binding 2)
			{ 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr },
			// Буфер трансформ (binding 3)
			{ 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr }
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &resources.descriptorSetLayout));

		// 2. Создание пула дескрипторов
		std::vector<VkDescriptorPoolSize> poolSizes = { { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4 },
														{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 } };

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 4;

		VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &resources.descriptorPool));


		// 4. Создание indirect буфера
		VkBufferCreateInfo indirectBufferInfo = {};
		indirectBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indirectBufferInfo.size = maxPatches * sizeof(DrawElementsIndirectCommand);
		indirectBufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		indirectBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		resources.descriptorSet.resize(3);
		resources.indirectBuffer.resize(3);
		resources.indirectMemory.resize(3);
		resources.counterBuffer.resize(3);
		resources.counterMemory.resize(3);

		for (size_t numImage = 0; numImage < 3; numImage++)
		{
			// 3. Выделение дескриптор сета
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = resources.descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &resources.descriptorSetLayout;
			VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &resources.descriptorSet[numImage]));

			VK_CHECK(vkCreateBuffer(device, &indirectBufferInfo, nullptr, &resources.indirectBuffer[numImage]));
			// Выделение памяти для indirect буфера
			VkMemoryRequirements indirectMemRequirements;
			vkGetBufferMemoryRequirements(device, resources.indirectBuffer[numImage], &indirectMemRequirements);

			VkMemoryAllocateInfo indirectAllocInfo = {};
			indirectAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			indirectAllocInfo.allocationSize = indirectMemRequirements.size;

			// Поиск подходящего типа памяти
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((indirectMemRequirements.memoryTypeBits & (1 << i)) &&
					(memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
				{
					indirectAllocInfo.memoryTypeIndex = i;
					break;
				}
			}

			VK_CHECK(vkAllocateMemory(device, &indirectAllocInfo, nullptr, &resources.indirectMemory[numImage]));
			VK_CHECK(
				vkBindBufferMemory(device, resources.indirectBuffer[numImage], resources.indirectMemory[numImage], 0)
			);

			// 5. Создание буфера-счетчика
			VkBufferCreateInfo counterBufferInfo = {};
			counterBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			counterBufferInfo.size = sizeof(uint32_t);
			counterBufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			counterBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VK_CHECK(vkCreateBuffer(device, &counterBufferInfo, nullptr, &resources.counterBuffer[numImage]));

			VkMemoryRequirements counterMemRequirements;
			vkGetBufferMemoryRequirements(device, resources.counterBuffer[numImage], &counterMemRequirements);

			VkMemoryAllocateInfo counterAllocInfo = {};
			counterAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			counterAllocInfo.allocationSize = counterMemRequirements.size;

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((counterMemRequirements.memoryTypeBits & (1 << i)) &&
					(memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
				{
					counterAllocInfo.memoryTypeIndex = i;
					break;
				}
			}

			VK_CHECK(vkAllocateMemory(device, &counterAllocInfo, nullptr, &resources.counterMemory[numImage]));
			VK_CHECK(vkBindBufferMemory(device, resources.counterBuffer[numImage], resources.counterMemory[numImage], 0));

			auto vertexBuffer = VulkanManagerBuffer::Get()->GetVertexBuffer(vertexBufferId)->buf_data->buffer;

			// 6. Обновление дескриптор сета
			std::vector<VkDescriptorBufferInfo> bufferInfos(4);

			// Вершинный буфер
			bufferInfos[0].buffer = vertexBuffer;
			bufferInfos[0].offset = 0;
			bufferInfos[0].range = VK_WHOLE_SIZE;

			// Indirect буфер
			bufferInfos[1].buffer = resources.indirectBuffer[numImage];
			bufferInfos[1].offset = 0;
			bufferInfos[1].range = VK_WHOLE_SIZE;

			// Буфер счетчик
			bufferInfos[2].buffer = resources.counterBuffer[numImage];
			bufferInfos[2].offset = 0;
			bufferInfos[2].range = VK_WHOLE_SIZE;

			// камера
			bufferInfos[3].buffer = VulkanManagerUniformBuffer::Get()->GetUniformBuffer(uniformBufferId)[numImage]->buf_data->buffer;
			bufferInfos[3].offset = 0;
			bufferInfos[3].range = VK_WHOLE_SIZE;

			std::vector<VkWriteDescriptorSet> descriptorWrites(bufferInfos.size());

			for (size_t i = 0; i < descriptorWrites.size(); i++)
			{
				descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[i].dstSet = resources.descriptorSet[numImage];
				descriptorWrites[i].dstBinding = static_cast<uint32_t>(i);
				descriptorWrites[i].dstArrayElement = 0;
				descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrites[i].descriptorCount = 1;
				descriptorWrites[i].pBufferInfo = &bufferInfos[i];
				if (i == 3)
				{
					descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				}
			}

			vkUpdateDescriptorSets(
				device,
				static_cast<uint32_t>(descriptorWrites.size()),
				descriptorWrites.data(),
				0,
				nullptr
			);
		}

		// 7. Создание compute шейдера (здесь нужно вставить SPIR-V код)
		// Предполагается, что у вас уже есть скомпилированный SPIR-V
		std::string name = "C:/cod/render/vulkan_renderer/assets/shaders/shader_water_v1.cmp";
		std::string glsl = ReaderShader::ReadFile(name);
		std::string spv = CompilerShaderModule::Compile(name, glsl, description::ShaderType::COMPUTE);

		VkShaderModuleCreateInfo shaderInfo = {};
		shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderInfo.codeSize = spv.size();
		shaderInfo.pCode = reinterpret_cast<const uint32_t*>(spv.data());
		;

		VK_CHECK(vkCreateShaderModule(device, &shaderInfo, nullptr, &resources.computeShaderModule));

		// 8. Создание pipeline layout с push constants
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(CullingPushConstants);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &resources.descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &resources.computePipelineLayout));

		// 9. Создание compute pipeline
		VkPipelineShaderStageCreateInfo stageInfo = {};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageInfo.module = resources.computeShaderModule;
		stageInfo.pName = "main";

		VkComputePipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = stageInfo;
		pipelineInfo.layout = resources.computePipelineLayout;

		VK_CHECK(
			vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resources.computePipeline)
		);

		return resources;
	}

	// Функция для добавления culling в командный буфер
	void VulkanRenderBase::addCullingToCommandBuffer(
		GraphicsWindowId idWindow,
		CommandBufferId commandBufferId,
		const CullingResources& resources,
		uint32_t currentPatchCount,
		const glm::vec3& cubeMin,
		const glm::vec3& cubeMax,
		size_t numImage
	)
	{
		auto commandBuffers = VulkanManagerCommandBuffer::Get()->GetCommandBuffer(idWindow, commandBufferId);

		auto& commandBuffer = commandBuffers[numImage];
		
		// 1. Обнуляем буфер-счетчик
		vkCmdFillBuffer(commandBuffer, resources.counterBuffer[numImage], 0, sizeof(uint32_t), 0);

		vkCmdFillBuffer(
			commandBuffer,
			resources.indirectBuffer[numImage],
			0,
			100 * sizeof(DrawElementsIndirectCommand),
			0
		);

		// 2. Барьер для обнуления
		VkBufferMemoryBarrier fillBarrier = {};
		fillBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		fillBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		fillBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		fillBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		fillBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		fillBarrier.buffer = resources.counterBuffer[numImage];
		fillBarrier.offset = 0;
		fillBarrier.size = sizeof(uint32_t);

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0,
			0,
			nullptr,
			1,
			&fillBarrier,
			0,
			nullptr
		);

		// 3. Биндинг compute pipeline
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, resources.computePipeline);
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			resources.computePipelineLayout,
			0,
			1,
			&resources.descriptorSet[numImage],
			0,
			nullptr
		);

		// 4. Push constants
		CullingPushConstants pc;
		pc.cubeMin = glm::vec4(cubeMin, 1.0f);
		pc.cubeMax = glm::vec4(cubeMax, 1.0f);
		pc.patchCount = currentPatchCount;

		vkCmdPushConstants(
			commandBuffer,
			resources.computePipelineLayout,
			VK_SHADER_STAGE_COMPUTE_BIT,
			0,
			sizeof(CullingPushConstants),
			&pc
		);

		// 5. Dispatch compute shader
		uint32_t groupCount = (currentPatchCount + 63) / 64; // 64 - local size
		vkCmdDispatch(commandBuffer, groupCount, 1, 1);

		// 6. Барьер для передачи результатов в graphics pipeline
		VkBufferMemoryBarrier computeToDrawBarrier = {};
		computeToDrawBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		computeToDrawBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		computeToDrawBarrier.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		computeToDrawBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		computeToDrawBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		computeToDrawBarrier.buffer = resources.indirectBuffer[numImage];
		computeToDrawBarrier.offset = 0;
		computeToDrawBarrier.size = VK_WHOLE_SIZE;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
			0,
			0,
			nullptr,
			1,
			&computeToDrawBarrier,
			0,
			nullptr
		);
	}

	CullingResources VulkanRenderBase::createMeshletRsources(
		GraphicsWindowId idWindow,
		uint32_t maxPatches,
		VertexBufferId vertexBufferId,
		UniformBufferId uniformBufferId
	)
	{ 
		return CullingResources();
	}

	VulkanRenderBase::VulkanRenderBase(
		std::vector<const char*> externalExtensions,
		std::shared_ptr<general::InterfaceManagerAssetRender> managerAsset)
		: manager_time_(ManagerTime::Get())
	{
		start_ = std::chrono::steady_clock::now();
		//timer_id_ = manager_time_.CreateTimer();

		LOG(Loglvl::info, "[VulkanRenderBase::VulkanRenderBase] start create VulkanRenderBase");

		ManagerAssetsVulkan::Get()->Init(managerAsset);

		initInstanceLayerProperties();

		createInstance(externalExtensions);

		init();
	}

	VulkanRenderBase::~VulkanRenderBase()
	{
		cleanup();
	}

	std::shared_ptr<DataInstance> VulkanRenderBase::GetInstance()
	{
		return instance_;
	}

	void VulkanRenderBase::DrawFrame()
	{
		VulkanManagerBuffer::Get()->DeletingBuffersForDeletedWindows();

		const auto& details = ManagerWindow::Get()->GetValidDetailWindows();
		for (const auto& detail : details)
		{
			const bool screenRendering = ManagerSwapchain::Get()->GetSwapchain(detail.swapchain_id) != nullptr;

			if (screenRendering)
			{
				drawFrameScreen(detail);
			}
			else
			{
				drawFrameOffScreen(detail);
			}
		}
	}

	void VulkanRenderBase::DeviceWaitIdle()
	{
		ManagerDevice::Get()->DeviceWaitIdle();
	}

	GraphicsWindowId VulkanRenderBase::AddWindow(
		const std::function<DataResult(std::shared_ptr<DataInstance>, std::shared_ptr<DataSurface>)>& createWindowSurface,
		uint32_t width,
		uint32_t height,
		float windowRatio,
		bool externalDeleter)
	{
		const SurfaceId surfaceId = ManagerSurface::Get()->AddSurface(
			instance_,
			createWindowSurface,
			externalDeleter);

		std::vector<const char*> extensions = requiredDeviceExtensions();

		const PhysicalDeviceId physicalDeviceId = ManagerDevice::Get()->GetPriorityPhysicalDeviceId();

		if (!ManagerDevice::Get()->IsPhysicalDeviceSuitable(
			physicalDeviceId,
			surfaceId,
			ManagerSurface::Get()->GetSurface(surfaceId),
			extensions))
		{
			LOGEXC(std::runtime_error, "[VulkanRenderBase::AddWindow] Physical device is not suitable!");
		}

		LogicalDeviceId logicalDeviceId = ManagerDevice::Get()->GetLogicalDeviceIdByPhysicalDeviceId(physicalDeviceId);
		if (!logicalDeviceId.IsValid())
		{
			logicalDeviceId = ManagerDevice::Get()->AddLogicalDevice(physicalDeviceId);
			ExtensionFunctions::Load(ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId));
		}

		auto swapchainId = ManagerSwapchain::Get()->AddSwapchain(
			surfaceId,
			logicalDeviceId,
			physicalDeviceId,
			width,
			height,
			windowRatio);

		auto windowId = ManagerWindow::Get()->AddWindow(
			swapchainId,
			surfaceId,
			physicalDeviceId,
			logicalDeviceId);

		auto addWindowInCommandBuff = VulkanManagerCommandBuffer::Get()->AddWindow(
			windowId,
			ManagerSwapchain::Get()->GetNumImages(swapchainId));

		if (!addWindowInCommandBuff)
		{
			LOGEXC(std::runtime_error, "[VulkanRenderBase::AddWindow] Failed to add the window with the existing id!");
		}

		return windowId;
	}

	GraphicsWindowId VulkanRenderBase::AddWindow(
		uint32_t width,
		uint32_t height,
		uint32_t numImages)
	{
		std::vector<const char*> extensions = requiredDeviceExtensions();

		const SurfaceId surfaceId = GeneratorId::GenerateInvalidId<SurfaceId>();
		const PhysicalDeviceId physicalDeviceId = ManagerDevice::Get()->GetPriorityPhysicalDeviceId();
		if (!ManagerDevice::Get()->IsPhysicalDeviceSuitable(
			physicalDeviceId,
			surfaceId,
			nullptr,
			extensions))
		{
			LOGEXC(std::runtime_error, "[VulkanRenderBase::AddWindow] Physical device is not suitable!");
		}

		LogicalDeviceId logicalDeviceId = ManagerDevice::Get()->GetLogicalDeviceIdByPhysicalDeviceId(physicalDeviceId);
		if (!logicalDeviceId.IsValid())
		{
			logicalDeviceId = ManagerDevice::Get()->AddLogicalDevice(physicalDeviceId);
			ExtensionFunctions::Load(ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId));
		}

		auto swapchainId = ManagerSwapchain::Get()->AddSwapchain(
			logicalDeviceId,
			physicalDeviceId,
			VkExtent2D(width, height),
			numImages);

		auto windowId = ManagerWindow::Get()->AddWindow(
			swapchainId,
			surfaceId,
			physicalDeviceId,
			logicalDeviceId);

		auto addWindowInCommandBuff = VulkanManagerCommandBuffer::Get()->AddWindow(
			windowId,
			ManagerSwapchain::Get()->GetNumImages(swapchainId));

		if (!addWindowInCommandBuff)
		{
			LOGEXC(std::runtime_error, "[VulkanRenderBase::AddWindow] Failed to add the window with the existing id!");
		}

		return windowId;
	}

	void VulkanRenderBase::DeleteWindow(const GraphicsWindowId& windowId)
	{
		VulkanManagerBuffer::Get()->AddDeletedWindow(windowId);
		auto surfaceId = ManagerWindow::Get()->GetSurfaceId(windowId);
		auto swapchainId = ManagerWindow::Get()->GetSwapchainId(windowId);
		auto framebufferId = VulkanManagerFrameBuffer::Get()->GetFrameBufferIdBySwapchainId(swapchainId);

		DeviceWaitIdle();

		VulkanManagerFrameBuffer::Get()->DeleteFramebuffer(framebufferId);
		ManagerSwapchain::Get()->DeleteSwapchain(swapchainId);
		ManagerSurface::Get()->DeleteSurface(surfaceId);
		ManagerWindow::Get()->RemoveWindow(windowId);
		GetManagerCommandBuffer()->DeleteWindowCommandBuffer(windowId);
	}

	void VulkanRenderBase::SetWindowRatio(const GraphicsWindowId& idWindow, float windowRatio)
	{
		const auto idSwachain = ManagerWindow::Get()->GetSwapchainId(idWindow);
		ManagerSwapchain::Get()->SetRatio(idSwachain, windowRatio);
	}

	[[nodiscard]] bool VulkanRenderBase::HasOneDevice(const GraphicsWindowId& windowId1, const GraphicsWindowId& windowId2)
	{
		return ManagerWindow::Get()->GetLogicalDeviceId(windowId1) == ManagerWindow::Get()->GetLogicalDeviceId(windowId2);
	}

	void VulkanRenderBase::Resize(const GraphicsWindowId& windowId, const uint32_t width, const uint32_t height)
	{
		auto idSwapchain = ManagerWindow::Get()->GetSwapchainId(windowId);
		ManagerSwapchain::Get()->Resize(idSwapchain, width, height);

		const bool offScreenFendering = ManagerSwapchain::Get()->GetSwapchain(idSwapchain) == nullptr;
		if (offScreenFendering)
		{
			DeviceWaitIdle();
			auto frameBufferId = VulkanManagerFrameBuffer::Get()->GetFrameBufferIdBySwapchainId(idSwapchain);
			VulkanManagerFrameBuffer::Get()->RecreateFrameBuffer(frameBufferId);
			GetManagerCommandBuffer()->RecreateCommandBuffers(windowId);
		}
	}

	CommandBufferId VulkanRenderBase::CreateCommandBuffer(const GraphicsWindowId& windowId)
	{
		return VulkanManagerCommandBuffer::Get()->CreateCommandBuffer(
			windowId,
			ManagerWindow::Get()->GetSwapchainId(windowId),
			ManagerWindow::Get()->GetLogicalDeviceId(windowId));
	}

	[[nodiscard]] std::shared_ptr<ManagerDrawcall> VulkanRenderBase::GetManagerDrawcall()
	{
		return VulkanManagerDrawcall::Get();
	}

	std::shared_ptr<ManagerCommandBuffer> VulkanRenderBase::GetManagerCommandBuffer()
	{
		return VulkanManagerCommandBuffer::Get();
	}

	std::shared_ptr<ManagerFrameBuffer> VulkanRenderBase::GetManagerFrameBuffer()
	{
		return VulkanManagerFrameBuffer::Get();
	}

	std::shared_ptr<ManagerTextures> VulkanRenderBase::GetManagerTextures()
	{
		return VulkanManagerTextures::Get();
	}

	std::shared_ptr<ManagerBuffer> VulkanRenderBase::GetManagerBuffer()
	{
		return VulkanManagerBuffer::Get();
	}

	std::shared_ptr<ManagerUniformBuffer> VulkanRenderBase::GetManagerUniformBuffer()
	{
		return VulkanManagerUniformBuffer::Get();
	}

	std::shared_ptr<ManagerPipeline> VulkanRenderBase::GetManagerPipeline()
	{
		return VulkanManagerPipeline::Get();
	}

	std::shared_ptr<ManagerShaderProgram> VulkanRenderBase::GetManagerShaderProgram()
	{
		return VulkanManagerShaderProgram::Get();
	}

	std::shared_ptr<ManagerRenderPass> VulkanRenderBase::GetManagerRenderPass()
	{
		return VulkanManagerRenderPass::Get();
	}

	const float VulkanRenderBase::GetAspect(const GraphicsWindowId& windowId) const
	{
		return ManagerSwapchain::Get()->GetAspect(ManagerWindow::Get()->GetSwapchainId(windowId));
	}

	const std::pair<uint32_t, uint32_t> VulkanRenderBase::GetWindowSize(const GraphicsWindowId& windowId) const
	{
		auto size = ManagerSwapchain::Get()->GetSize(ManagerWindow::Get()->GetSwapchainId(windowId));
		return { size.x, size.y };
	}

	const std::shared_ptr<general::InterfaceManagerAssetRender> VulkanRenderBase::GetManagerAsset() const
	{
		return ManagerAssetsVulkan::GetInterface();
	}

	const std::string VulkanRenderBase::GetNamePhysicalDevice(const GraphicsWindowId& windowId) const
	{
		return ManagerDevice::Get()->GetVkPhysicalDeviceProperties(ManagerWindow::Get()->GetPhysicalDeviceId(windowId)).deviceName;
	}


	std::shared_ptr<image::InterfaceImage> VulkanRenderBase::GetWindowPixelData(const GraphicsWindowId& windowId, bool alpha, FrameBufferId frameBufferId)
	{
		return ManagerWindow::Get()->GetWindowPixelData(windowId, alpha, frameBufferId);
	}

	void VulkanRenderBase::init()
	{
		// Needs VK_EXT_debug_utils on the instance, which createInstance only requests in a
		// debug build (see CreatorInstance::CreateInstace) - stays null otherwise.
#ifdef _DEBUG
		debug_utils_messenger_ = CreatorDebugUtilMessenger::CreateDebugUtilsMessenger(instance_);
#endif

		manager_window_ = ManagerWindow::Get();
		manager_device_ = ManagerDevice::Get();
		manager_device_->Init(instance_);
		manager_surface_ = ManagerSurface::Get();
		manager_swapchain_ = ManagerSwapchain::Get();

		manager_buffer_ = VulkanManagerBuffer::Get();
		manager_command_buffer_ = VulkanManagerCommandBuffer::Get();
		manager_drawcall_ = VulkanManagerDrawcall::Get();
		manager_frame_buffer_ = VulkanManagerFrameBuffer::Get();
		manager_pipeline_ = VulkanManagerPipeline::Get();
		manager_render_pass_ = VulkanManagerRenderPass::Get();
		manager_shader_program_ = VulkanManagerShaderProgram::Get();
		manager_sampler_ = ManagerSampler::Get();
		manager_textures_ = VulkanManagerTextures::Get();
		manager_uniform_buffer_ = VulkanManagerUniformBuffer::Get();

		printInfo();
	}

	void VulkanRenderBase::initInstanceExtensions()
	{
		uint32_t extensionCount = 0;
		auto res = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		if (res != VK_SUCCESS)
		{
			LOGEXC(std::runtime_error, "[VulkanRenderBase::initInstanceExtensions]");
		}

		instance_extensions_.resize(extensionCount);
		res = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, instance_extensions_.data());
		if (res != VK_SUCCESS)
		{
			LOGEXC(std::runtime_error, "[VulkanRenderBase::initInstanceExtensions]");
		}
	}

	void VulkanRenderBase::initInstanceLayerProperties()
	{
		// The validation layer is a development tool that ships with the Vulkan SDK, not with
		// the GPU driver - a player's machine has the Vulkan runtime to run this, but not the
		// SDK, so requesting it unconditionally would make vkCreateInstance fail there with
		// VK_ERROR_LAYER_NOT_PRESENT. Debug-only, and even then...
#ifdef _DEBUG
		validation_layers_ = {
			"VK_LAYER_KHRONOS_validation"
		};
#endif
		available_layers_ = ValidationLayer::initValidationLayerSupport(validation_layers_);

		// ...only request whichever of the above the loader actually reports, so a Debug
		// build still runs on a machine that also lacks the SDK, instead of hard failing.
		std::erase_if(validation_layers_, [this](const char* layerName)
		{
			const bool isAvailable = std::any_of(
				available_layers_.begin(), available_layers_.end(),
				[&](const VkLayerProperties& layer) { return std::strcmp(layerName, layer.layerName) == 0; });

			if (!isAvailable)
			{
				LOG(Loglvl::warning, "[VulkanRenderBase::initInstanceLayerProperties] requested layer not available, skipping:", layerName);
			}

			return !isAvailable;
		});
	}

	void VulkanRenderBase::createInstance(std::vector<const char*>& extensions)
	{
		external_extensions_ = extensions;

		initInstanceExtensions();

		instance_ = CreatorInstance::CreateInstace(extensions, validation_layers_);
	}

	void VulkanRenderBase::cleanup()
	{
		DeviceWaitIdle();

		ManagerWindow::Get() = nullptr;

		VulkanManagerCommandBuffer::Get() = nullptr;
		VulkanManagerBuffer::Get() = nullptr;
		VulkanManagerDrawcall::Get() = nullptr;
		VulkanManagerFrameBuffer::Get() = nullptr;
		VulkanManagerPipeline::Get() = nullptr;
		VulkanManagerRenderPass::Get() = nullptr;
		VulkanManagerShaderProgram::Get() = nullptr;
		VulkanManagerTextures::Get() = nullptr;
		VulkanManagerUniformBuffer::Get() = nullptr;

		ManagerSampler::Get() = nullptr;

		ManagerSurface::Get() = nullptr;
		ManagerDevice::Get() = nullptr;
		ManagerSwapchain::Get() = nullptr;
	}

	void VulkanRenderBase::recreateSwapChain(const GraphicsWindowId& windowId)
	{
		DeviceWaitIdle();

		auto swapchainId = ManagerWindow::Get()->GetSwapchainId(windowId);
		ManagerSwapchain::Get()->RecreateSwapchain(swapchainId);

		// The ray tracing pass writes straight into the swapchain images, so its descriptors
		// refer to image views that the recreation above has just replaced.
		ManagerRayTracing::Get()->ApplyResize(swapchainId);

		// A ray traced window has no frame buffer at all, so only refresh one if it exists.
		auto frameBufferId = VulkanManagerFrameBuffer::Get()->FindFrameBufferIdBySwapchainId(swapchainId);
		if (frameBufferId.IsValid())
		{
			VulkanManagerFrameBuffer::Get()->RecreateFrameBuffer(frameBufferId);
		}

		GetManagerCommandBuffer()->RecreateCommandBuffers(windowId);
	}

	void VulkanRenderBase::printInfo()
	{
		LOG(Loglvl::info, "available extensions:");
		for (const auto& extension : instance_extensions_) {
			LOG(Loglvl::info, '\t' , extension.extensionName);
		}


		LOG(Loglvl::info, "Available Layers:");
		for (const auto& layer : available_layers_) {
			LOG(Loglvl::info, '\t' , layer.layerName);
		}


		LOG(Loglvl::info, "Validation Layers:");
		for (const auto& layer : validation_layers_) {
			LOG(Loglvl::info, '\t' , layer);
		}
	}

	void VulkanRenderBase::drawFrameScreen(const DetailWindow& detail)
	{
		// TO DO
		// check window activity
		if (VulkanManagerCommandBuffer::Get()->GetNumberCommandBuffers(detail.id_window) == 0)
		{
			return;
		}

		const auto& swapchainId = detail.swapchain_id;
		const auto& logicalDeviceId = detail.logical_device_id;
		const auto& logicalDevice = ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId);
		const auto& graphicQueue = ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId);

		const uint32_t frameIndex = ManagerSwapchain::Get()->GetCurrentFrameIndex(swapchainId);
		const auto& fence = ManagerSwapchain::Get()->GetFence(swapchainId, frameIndex);

		vkWaitForFences(logicalDevice->device, 1, &fence->fence, VK_TRUE, UINT64_MAX);
		vkResetFences(logicalDevice->device, 1, &fence->fence);

		const std::shared_ptr<DataSemaphore>& availableSemaphore = ManagerSwapchain::Get()->GetAvailableSemaphore(swapchainId, frameIndex);

		uint32_t imageIndex = 0;
		auto result = vkAcquireNextImageKHR(
			logicalDevice->device,
			ManagerSwapchain::Get()->GetSwapchain(swapchainId)->swapchain,
			UINT64_MAX,
			availableSemaphore->semaphore,
			VK_NULL_HANDLE,
			&imageIndex);

		ManagerSwapchain::Get()->SetCurrentImageIndex(detail.swapchain_id, imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain(detail.id_window);
			return;
		}
		else if (result == VK_SUBOPTIMAL_KHR)
		{
			// TO DO
		}
		else if (result != VK_SUCCESS)
		{
			LOGEXC(std::runtime_error, "failed to vkAcquireNextImageKHR:", int(result));
		}

		// The frame fence above only says that this frame slot is free. The command buffer
		// about to be submitted belongs to the image, so if another frame is still using
		// that image, wait for it too. Skipped when it is the fence just waited on.
		const VkFence imageInFlight = ManagerSwapchain::Get()->GetImageInFlightFence(swapchainId, imageIndex);
		if (imageInFlight != VK_NULL_HANDLE && imageInFlight != fence->fence)
		{
			vkWaitForFences(logicalDevice->device, 1, &imageInFlight, VK_TRUE, UINT64_MAX);
		}
		ManagerSwapchain::Get()->SetImageInFlightFence(swapchainId, imageIndex, fence->fence);

		// Command buffer slot i is recorded against swapchain image i: it transitions that
		// image and binds the descriptor set that writes into it. The acquired image index
		// is not always the frame index, so the slot has to follow the image, not the frame.
		const auto& commandBuffers = VulkanManagerCommandBuffer::Get()->GetCommandBuffersByNumImage(detail.id_window, imageIndex);
		VulkanManagerBuffer::Get()->ProcessDeletingBuffers(detail.id_window);

		VulkanManagerUniformBuffer::Get()->UpdateSwapchainUniformBuffers(swapchainId, imageIndex);

		// Now that the image is known, the frame's uniform block can go into that image's buffer.
		ManagerRayTracing::Get()->UploadUniform(swapchainId, imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { availableSemaphore->semaphore };
		VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = &waitStages;

		submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
		submitInfo.pCommandBuffers = commandBuffers.data();

		VkSemaphore signalSemaphores[] = { ManagerSwapchain::Get()->GetFinishedSemaphore(swapchainId, imageIndex)->semaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		result = vkQueueSubmit(graphicQueue, 1, &submitInfo, fence->fence);

		if (result != VK_SUCCESS)
		{
			LOGEXC(std::runtime_error, "[VulkanRenderBase::drawFrameScreen] failed to submit draw command buffer");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { ManagerSwapchain::Get()->GetSwapchain(swapchainId)->swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		presentInfo.pResults = nullptr; // Optional

		result = vkQueuePresentKHR(graphicQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			recreateSwapChain(detail.id_window);
		}
		else if (result != VK_SUCCESS)
		{
			LOGEXC(std::runtime_error, "[VulkanRenderBase::drawFrameScreen] failed to present swap chain image!");
		}

		ManagerSwapchain::Get()->SetCurrentFrameIndex(swapchainId, (frameIndex + 1) % (uint32_t)ManagerSwapchain::Get()->GetNumFramesInFlight(swapchainId));
	}

	void VulkanRenderBase::drawFrameOffScreen(const DetailWindow& detail)
	{
		// TO DO
		// check window activity
		auto managerCommandBuffer = VulkanManagerCommandBuffer::Get();
		if (managerCommandBuffer->GetNumberCommandBuffers(detail.id_window) == 0)
		{
			return;
		}

		const auto& swapchainId = detail.swapchain_id;
		const auto& logicalDeviceId = detail.logical_device_id;
		const auto& logicalDevice = ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId);
		const auto& graphicQueue = ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId);

		uint32_t frameIndex = (ManagerSwapchain::Get()->GetCurrentFrameIndex(swapchainId));
		// Since we don't have an image acquisition function here, due to the lack of a surface to present to,
		// we have to emulate the acquisition function by simply assigning the image index to the frame index.
		uint32_t imageIndex = frameIndex;
		ManagerSwapchain::Get()->SetCurrentImageIndex(swapchainId, imageIndex);

		auto fence = ManagerSwapchain::Get()->GetFence(swapchainId, frameIndex);

		vkWaitForFences(logicalDevice->device, 1, &fence->fence, VK_TRUE, UINT64_MAX);
		vkResetFences(logicalDevice->device, 1, &fence->fence);

		const auto& commandBuffers = VulkanManagerCommandBuffer::Get()->GetCommandBuffersByNumImage(detail.id_window, frameIndex);
		VulkanManagerBuffer::Get()->ProcessDeletingBuffers(detail.id_window);

		VulkanManagerUniformBuffer::Get()->UpdateSwapchainUniformBuffers(swapchainId, frameIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = &waitStages;

		submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
		submitInfo.pCommandBuffers = commandBuffers.data();

		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;

		const VkResult result = vkQueueSubmit(graphicQueue, 1, &submitInfo, fence->fence);
		if (result != VK_SUCCESS)
		{
			LOGEXC(std::runtime_error, "[VulkanRenderBase::drawFrameOffScreen] failed to submit draw command buffer");
		}

	ManagerSwapchain::Get()->SetCurrentFrameIndex(swapchainId, (frameIndex + 1) % ManagerSwapchain::Get()->GetNumFramesInFlight(swapchainId));
}
}
