#include <algorithm>

#include "vulkan_manager_command_buffer.h"
#include "creator_command_buffer.h"
#include "creator_pipeline_layout.h"
#include "../manager_swapchain.h"
#include "../manager_device.h"

namespace render
{
    std::shared_ptr<VulkanManagerCommandBuffer>& VulkanManagerCommandBuffer::Get()
    {
        static std::shared_ptr<VulkanManagerCommandBuffer> manager;
        if (!manager || manager->NeedReinit())
        {
            manager = std::shared_ptr<VulkanManagerCommandBuffer>(new VulkanManagerCommandBuffer());
        }
        return manager;
    }

    VulkanManagerCommandBuffer::~VulkanManagerCommandBuffer()
    {
        LOG(Loglvl::debug, "[VulkanManagerCommandBuffer::~VulkanManagerCommandBuffer]");
    }

    size_t VulkanManagerCommandBuffer::GetNumberCommandBuffers(const GraphicsWindowId& windowId)
    {
        auto buffer = command_buffers_.find(windowId);
        if (buffer == command_buffers_.end())
        {
            return 0;
        }
        return buffer->second->GetNumCommandBufferForDraw(0);
    }

    std::vector<VkCommandBuffer> VulkanManagerCommandBuffer::GetCommandBuffer(
		const GraphicsWindowId& windowId,
		const CommandBufferId& commandBufferId
	)
    {
		auto buffer = command_buffers_.find(windowId);
		if (buffer == command_buffers_.end())
		{
			return {};
		}
		return buffer->second->GetCommandBuffer(commandBufferId);
    }

    std::vector<VkCommandBuffer> VulkanManagerCommandBuffer::GetCommandBuffersByNumImage(
        const GraphicsWindowId& windowId,
        const size_t numImage)
    {
        auto buffer = command_buffers_.find(windowId);
        if (buffer == command_buffers_.end())
        {
            return {};
        }
        return buffer->second->GetCommandBufferForDraw(numImage);
    }

    bool VulkanManagerCommandBuffer::AddWindow(
        const GraphicsWindowId& windowId,
        const size_t numSwapchainImages)
    {
        if (command_buffers_.count(windowId) != 0)
        {
            return false;
        }

        command_buffers_[windowId] = std::make_shared<CommandBuffer>(numSwapchainImages);

        return true;
    }

    CommandBufferId VulkanManagerCommandBuffer::CreateCommandBuffer(
        const GraphicsWindowId& windowId,
        const SwapchainId& swapchainId,
        const LogicalDeviceId& logicalDeviceId)
    {
        auto newId = command_buffers_[windowId]->CreateCommandBuffer(
            ManagerDevice::Get()->GetCommandPool(logicalDeviceId),
            ManagerSwapchain::Get()->GetExtent(swapchainId));

        return newId;
    }

    void VulkanManagerCommandBuffer::AddDrawcallInCommandBuffer(const GraphicsWindowId& windowId, const DrawcallId& drawcallId, const CommandBufferId& commandBufferId)
    {
        command_buffers_[windowId]->AddDrawcallInCommandBuffer(drawcallId, commandBufferId);
    }

    void VulkanManagerCommandBuffer::RecreateCommandBuffers(const GraphicsWindowId& windowId)
    {
        command_buffers_[windowId]->RecreateCommandBuffers();
    }

    void VulkanManagerCommandBuffer::AddToDrawingQueue(
        const GraphicsWindowId& windowId,
        const CommandBufferId& commandBufferId, 
        const DrawPriority drawPriority)
    {
        command_buffers_[windowId]->AddToDrawingQueue(commandBufferId, drawPriority);
    }

    void VulkanManagerCommandBuffer::RecreateCommandBuffer(const GraphicsWindowId& windowId, const CommandBufferId& commandBufferId)
    {
        auto find = command_buffers_.find(windowId);
        if (find != command_buffers_.end())
        {
            find->second->RecreateCommandBuffer(commandBufferId);
        }
    }

    void VulkanManagerCommandBuffer::DeleteFromDrawingQueue(
        const GraphicsWindowId& windowId,
        const CommandBufferId& commandBufferId)
    {
        if (auto currentComandBuffer = command_buffers_.find(windowId); currentComandBuffer != command_buffers_.end())
        {
            currentComandBuffer->second->RemoveFromDrawingQueue(commandBufferId);
        }
    }

    bool VulkanManagerCommandBuffer::DeleteDrawCallFromCommandBuffer(const GraphicsWindowId& windowId, DrawcallId& drawcallId)
    {
        if (!windowId.IsValid() || !drawcallId.IsValid())
        {
            LOGEXC(std::runtime_error, "[VulkanManagerCommandBuffer::DeleteDrawCallFromCommandBuffer] invalid id");
        }

        auto currentBuffer = command_buffers_.find(windowId);
        if (currentBuffer == command_buffers_.end())
        {
            return false;
        }

        return currentBuffer->second->DeleteBindingFromBuffer(drawcallId);
    }

    bool VulkanManagerCommandBuffer::DeleteCommandBuffer(
        const GraphicsWindowId& windowId,
        const CommandBufferId& commandBufferId)
    {
        auto currentPair = command_buffers_.find(windowId);
        if (currentPair == command_buffers_.end())
        {
            return false;
        }

        auto currentBuffer = currentPair->second;

        currentBuffer->DeleteCommandBuffer(commandBufferId);

        return true;
    }

    void VulkanManagerCommandBuffer::DeleteWindowCommandBuffer(const GraphicsWindowId& windowId)
    {
        command_buffers_.erase(windowId);
    }
}
