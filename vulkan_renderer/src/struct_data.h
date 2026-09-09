#pragma once

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace render
{
	struct DataInstance;
	struct DataSurface;
	struct DataResult;

struct DataDebugUtilsMessenger
{
	VkDebugUtilsMessengerEXT debug_utils_messenger;
	std::shared_ptr<DataInstance> instance;
};

struct DataSwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities = VkSurfaceCapabilitiesKHR{};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

struct DataDevice
{
	VkDevice device;
	std::shared_ptr<DataInstance> instance;
	VmaAllocator allocator = VK_NULL_HANDLE;
};

struct DataSwapchain
{
	VkSurfaceFormatKHR format;
	VkSwapchainKHR swapchain;
	std::shared_ptr<DataDevice> device;
	std::shared_ptr<DataSurface> surface;
};

struct DataImageView
{
	VkImageView image_view;
	std::shared_ptr<DataDevice> device;
};

struct DataShaderModule
{
	VkShaderModule shader;
	std::shared_ptr<DataDevice> device;
};

struct DataShader
{
	std::filesystem::path filepath;
	VkShaderStageFlagBits stage;
	std::shared_ptr<DataShaderModule> shader_module_data;
	std::vector<VkVertexInputBindingDescription> input_binding_descriptions;
	std::vector<VkVertexInputAttributeDescription> input_attribute_descriptions;
};

struct DataPipelineLayout
{
	VkPipelineLayout pipline_layout;
	std::shared_ptr<DataDevice> device;
};

struct DataRenderPass
{
	VkRenderPass render_pass;
	std::shared_ptr<DataDevice> device;
};

struct DataPipeline
{
	VkPipeline pipeline;
	std::shared_ptr<DataDevice> device;
};

struct DataFrameBuffer
{
	VkFramebuffer framebuffer;
	std::shared_ptr<DataDevice> device;
	VkExtent2D extent;
};

struct DataCommandPool
{
	VkCommandPool command_pool;
	std::shared_ptr<DataDevice> device;
};

struct DataSemaphore
{
	VkSemaphore semaphore;
	std::shared_ptr<DataDevice> device;
};

struct DataFence
{
	VkFence fence;
	std::shared_ptr<DataDevice> device;
};

struct DataDeviceMemory
{
	uint64_t size;
	VmaAllocation allocation = VK_NULL_HANDLE;
	std::shared_ptr<DataDevice> device;
};

struct DataBuffer
{
	void* mapped_data = nullptr;
	uint64_t size;
	VkBuffer buffer;
	std::shared_ptr<DataDevice> device;
};

struct DataVertexBuffer
{
	std::shared_ptr<DataBuffer> buf_data;
	std::shared_ptr<DataDeviceMemory> device_memory;
};

struct DataIndexBuffer
{
	VkIndexType index_type;
	uint32_t num_index;
	std::shared_ptr<DataBuffer> buf_data;
	std::shared_ptr<DataDeviceMemory> device_memory;
};

struct DataUniformBuffer
{
	std::shared_ptr<DataBuffer> buf_data;
	std::shared_ptr<DataDeviceMemory> device_memory;
};

struct DataDescriptorSetLayout
{
	VkDescriptorSetLayout descriptor_set_layout;
	std::shared_ptr<DataDevice> device;
};

struct DataDescriptorPool
{
	VkDescriptorPool descriptor_pool;
	std::shared_ptr<DataDevice> device;
};

struct DataImage
{
	VkImage image;
	std::shared_ptr<DataDevice> device;
};

struct DataSampler
{
	VkSampler sampler;
	std::shared_ptr<DataDevice> device;
};

struct DataTexture
{
	// TO DO
	std::shared_ptr<DataDevice> device;
};

/*!
\brief A built acceleration structure together with the memory it lives in.

The device address is what a top level structure stores for each of its instances and what
the shader binding table resolves, so it is cached here instead of being queried per use.
*/
struct DataAccelerationStructure
{
	VkAccelerationStructureKHR acceleration_structure = VK_NULL_HANDLE;
	VkDeviceAddress device_address = 0;
	std::shared_ptr<DataBuffer> buffer;
	std::shared_ptr<DataDeviceMemory> device_memory;
	std::shared_ptr<DataDevice> device;
};

}
