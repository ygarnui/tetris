#include "creator_command_buffer.h"
#include "creator_graphics_pipeline.h"

#include <logger_instance.h>

#include <stdexcept>
#include <array>

namespace render
{

std::vector<VkCommandBuffer> CreatorCommandBuffer::CreateCommandBuffer(
	std::shared_ptr<DataCommandPool> commandPool,
	const VkExtent2D& extent,
	const size_t numSwapchainImages)
{
	std::vector<VkCommandBuffer> commandBuffer(numSwapchainImages);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool->command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(numSwapchainImages);

	auto res = vkAllocateCommandBuffers(commandPool->device->device, &allocInfo, commandBuffer.data());

	if (res != VK_SUCCESS) 
	{
		LOGEXC(std::runtime_error, "[CreatorCommandBuffer::CreateCommandBuffer] failed to allocate command buffers");
	}

	return commandBuffer;
}

void CreatorCommandBuffer::EndCommandBuffer(VkCommandBuffer commandBuffer)
{
	auto res = vkEndCommandBuffer(commandBuffer);
	if (res != VK_SUCCESS) 
	{
		LOGEXC(std::runtime_error, "[CreatorCommandBuffer::EndCommandBuffer] failed to record command buffer");
	}
}

void CreatorCommandBuffer::ResetCommandBuffer(VkCommandBuffer commandBuffer)
{
	auto res = vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	if (res != VK_SUCCESS)
	{
        LOGEXC(std::runtime_error, "failed to reset command buffer");
	}
}

void CreatorCommandBuffer::RecreateCommandBuffer(
	VkCommandBuffer commandBuffer,
	std::shared_ptr<DataCommandPool> commandPool,
	const VkExtent2D& extent)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = VK_NULL_HANDLE; // Optional

	auto res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	if (res != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorCommandBuffer::RecreateCommandBuffer] failed to begin recording command buffer");
	}

	VkViewport viewport = CreatorGraphicsPipeline::CreateViewport(extent);

	VkRect2D scissor = CreatorGraphicsPipeline::CreateRect2D(extent);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

}
