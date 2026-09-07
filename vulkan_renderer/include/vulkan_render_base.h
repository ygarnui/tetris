#pragma once

#include "../src/struct_data.h"
#include "../src/struct_base_data.h"
#include "../src/device_property.h"
#include "../src/manager_window.h"
#include "../src/vulkan_manager_frame_buffer.h"
#include "../src/buffers/vulkan_manager_buffer.h"
#include "../src/pipeline/vulkan_manager_command_buffer.h"
#include "../src/pipeline/vulkan_manager_drawcall.h"
#include "../src/shader/vulkan_manager_shader_program.h"
#include "../src/textures/vulkan_manager_textures.h"
#include "../src/utils/manager_time.h"
#include "../src/utils/guard_time.h"

#include <render_base.h>
#include <vulkan/vulkan.h> 
#include <interface_image_object.h>

#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <filesystem>
#include <optional>

namespace render 
{

	struct CullingResources
	{
		VkPipeline computePipeline;
		VkPipelineLayout computePipelineLayout;
		VkDescriptorSetLayout descriptorSetLayout;
		std::vector<VkDescriptorSet> descriptorSet;
		VkDescriptorPool descriptorPool;
		std::vector<VkBuffer> indirectBuffer;
		std::vector<VkDeviceMemory> indirectMemory;
		std::vector<VkBuffer> counterBuffer;
		std::vector<VkDeviceMemory> counterMemory;
		VkShaderModule computeShaderModule;
		uint32_t maxPatches;
	};

	// Push constants структура
	struct CullingPushConstants
	{
		glm::vec4 cubeMin;
		glm::vec4 cubeMax;
		uint32_t patchCount;
	};

	// Структура для indirect команды
	struct DrawElementsIndirectCommand
	{
		uint32_t indexCount;	// 4 для квада
		uint32_t instanceCount; // 1
		uint32_t firstIndex;	// первый индекс патча
		int32_t vertexOffset;	// 0
		uint32_t firstInstance; // 0
	};

class VulkanRenderBase : public RenderBase
{
public:

	CullingResources createCullingResources(
		GraphicsWindowId idWindow,
		uint32_t maxPatches,
		VertexBufferId vertexBufferId,
		UniformBufferId uniformBufferId
	);

	void addCullingToCommandBuffer(
		GraphicsWindowId idWindow,
		CommandBufferId commandBufferId,
		const CullingResources& resources,
		uint32_t currentPatchCount,
		const glm::vec3& cubeMin,
		const glm::vec3& cubeMax,
		size_t numImage
	);

	CullingResources createMeshletRsources(
		GraphicsWindowId idWindow,
		uint32_t maxPatches,
		VertexBufferId vertexBufferId,
		UniformBufferId uniformBufferId
	);

	VulkanRenderBase(
		std::vector<const char*> externalExtensions,
		std::shared_ptr<general::InterfaceManagerAssetRender> managerAsset);

	virtual ~VulkanRenderBase();

	std::shared_ptr<DataInstance> GetInstance() override;

	void DrawFrame() override;

	void DeviceWaitIdle() override;

	[[nodiscard]] GraphicsWindowId AddWindow(
		const std::function<DataResult(std::shared_ptr<DataInstance>, std::shared_ptr<DataSurface>)>& createWindowSurface,
		uint32_t width,
		uint32_t height,
		float windowRatio,
		bool externalDeleter) override;

	[[nodiscard]] GraphicsWindowId AddWindow(
		uint32_t width,
		uint32_t height,
		uint32_t numImages) override;

	void DeleteWindow(const GraphicsWindowId& windowId) override;

	void SetWindowRatio(const GraphicsWindowId& idWindow, float windowRatio) override;

	[[nodiscard]] bool HasOneDevice(const GraphicsWindowId& windowId1, const GraphicsWindowId& windowId2) override;

	void Resize(const GraphicsWindowId& windowId, const uint32_t width, const uint32_t height) override;

	[[nodiscard]] CommandBufferId CreateCommandBuffer(const GraphicsWindowId& windowId) override;

	[[nodiscard]] virtual std::shared_ptr<ManagerBuffer> GetManagerBuffer() override;
	[[nodiscard]] virtual std::shared_ptr<ManagerCommandBuffer> GetManagerCommandBuffer() override;
	[[nodiscard]] virtual std::shared_ptr<ManagerDrawcall> GetManagerDrawcall() override;
	[[nodiscard]] virtual std::shared_ptr<ManagerFrameBuffer> GetManagerFrameBuffer() override;
	[[nodiscard]] virtual std::shared_ptr<ManagerPipeline> GetManagerPipeline() override;
	[[nodiscard]] virtual std::shared_ptr<ManagerRenderPass> GetManagerRenderPass() override;
	[[nodiscard]] virtual std::shared_ptr<ManagerShaderProgram> GetManagerShaderProgram() override;
	[[nodiscard]] virtual std::shared_ptr<ManagerTextures> GetManagerTextures() override;
	[[nodiscard]] virtual std::shared_ptr<ManagerUniformBuffer> GetManagerUniformBuffer() override;

	[[nodiscard]] const float GetAspect(const GraphicsWindowId& windowId) const override;
	[[nodiscard]] const std::pair<uint32_t, uint32_t> GetWindowSize(const GraphicsWindowId& windowId) const override;

	[[nodiscard]] const std::shared_ptr<general::InterfaceManagerAssetRender> GetManagerAsset() const override;

	[[nodiscard]] const std::string GetNamePhysicalDevice(const GraphicsWindowId& windowId) const override;

	[[nodiscard]] std::shared_ptr<image::InterfaceImage> GetWindowPixelData(const GraphicsWindowId& windowId, bool alpha, FrameBufferId frameBufferId) override;

private:
	std::shared_ptr<DataInstance> instance_;

	void init();

	void initInstanceExtensions();

	void initInstanceLayerProperties();

	void createInstance(std::vector<const char*>& extensions);
	
	void cleanup();

	void recreateSwapChain(const GraphicsWindowId& windowId);

	void printInfo();

	void drawFrameScreen(const render::DetailWindow& detail);

	void drawFrameOffScreen(const render::DetailWindow& detail);
	
	std::shared_ptr<DataDebugUtilsMessenger> debug_utils_messenger_;

	std::vector<VkExtensionProperties> instance_extensions_;
	std::vector<VkLayerProperties> available_layers_;

	std::vector<const char*> validation_layers_;
	std::vector<const char*> external_extensions_;

	std::vector<const char*> device_extensions_;

	std::shared_ptr<ManagerWindow> manager_window_;

	std::shared_ptr<VulkanManagerBuffer> manager_buffer_;
	std::shared_ptr<VulkanManagerCommandBuffer> manager_command_buffer_;
	std::shared_ptr<VulkanManagerDrawcall> manager_drawcall_;
	std::shared_ptr<VulkanManagerFrameBuffer> manager_frame_buffer_;
	std::shared_ptr<VulkanManagerPipeline> manager_pipeline_;
	std::shared_ptr<VulkanManagerRenderPass> manager_render_pass_;
	std::shared_ptr<VulkanManagerShaderProgram> manager_shader_program_;
	std::shared_ptr<VulkanManagerTextures> manager_textures_;
	std::shared_ptr<ManagerSampler> manager_sampler_;
	std::shared_ptr<VulkanManagerUniformBuffer> manager_uniform_buffer_;

	std::shared_ptr<ManagerSurface> manager_surface_;
	std::shared_ptr<ManagerDevice> manager_device_;
	std::shared_ptr<ManagerSwapchain> manager_swapchain_;

	ManagerTime& manager_time_;
	size_t timer_id_;
	std::chrono::steady_clock::time_point start_;
};

}