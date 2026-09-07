#include "vulkan_manager_buffer.h"
#include "creator_buffer.h"
#include "creator_vertex_buffer.h"
#include "creator_index_buffer.h"
#include "creator_uniform_buffer.h"
#include "../manager_swapchain.h"

#include <guard_next_id.h>

#include "logger_instance.h"

namespace render
{
	VulkanManagerBuffer::VulkanManagerBuffer()
	{
	}

	std::shared_ptr<VulkanManagerBuffer>& VulkanManagerBuffer::Get()
	{
		static std::shared_ptr<VulkanManagerBuffer> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<VulkanManagerBuffer>(new VulkanManagerBuffer());
		}
		return manager;
	}

	VulkanManagerBuffer::~VulkanManagerBuffer()
	{
		LOG(Loglvl::debug, "[VulkanManagerBuffer::~VulkanManagerBuffer]");
	}

	VertexBufferId VulkanManagerBuffer::CreateVertexBuffer(
		const GraphicsWindowId& windowId,
		const uint64_t size,
		const description::MemoryAccess memoryAccess)
	{
		return createVertexBuffer(
			size,
			memoryAccess,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerWindow::Get()->GetPhysicalDeviceId(windowId));
	}

	void VulkanManagerBuffer::WriteToVertexBuffer(
		const VertexBufferId& vertexBufferId,
		const GraphicsWindowId& windowId,
		const void* data, const uint64_t size,
		const uint64_t offset)
	{
		return writeToVertexBuffer(
			vertexBufferId,
			data,
			size,
			offset,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerWindow::Get()->GetPhysicalDeviceId(windowId));
	}

	void VulkanManagerBuffer::ResizeVertexBuffer(
		const VertexBufferId& vertexBufferId,
		const GraphicsWindowId& windowId,
		const uint64_t size)
	{
		return resizeVertexBuffer(
			vertexBufferId,
			size,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerWindow::Get()->GetPhysicalDeviceId(windowId));
	}

	IndexBufferId VulkanManagerBuffer::CreateIndexBuffer(
		const GraphicsWindowId& windowId,
		const uint64_t size,
		const description::IndexType type)
	{
		return createIndexBuffer(
			size,
			type,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerWindow::Get()->GetPhysicalDeviceId(windowId));
	}

	void VulkanManagerBuffer::WriteToIndexBuffer(
		const IndexBufferId& indexBufferId,
		const GraphicsWindowId& windowId,
		const void* data,
		const uint64_t size,
		const uint64_t offset)
	{
		return writeToIndexBuffer(
			indexBufferId,
			data,
			size,
			offset,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerWindow::Get()->GetPhysicalDeviceId(windowId));
	}

	void VulkanManagerBuffer::ResizeIndexBuffer(
		const IndexBufferId& indexBufferId,
		const GraphicsWindowId& windowId,
		const uint64_t size)
	{
		auto indexBuffer = GetIndexBuffer(indexBufferId);
		auto deviceId = ManagerWindow::Get()->GetLogicalDeviceId(windowId);
		auto device = ManagerDevice::Get()->GetLogicalDevice(deviceId);
		if (device != indexBuffer->buf_data->device)
		{
			LOGEXC(std::invalid_argument, "this index buffer is not present on this device");
		}

		ManagerDevice::Get()->DeviceWaitIdle();

		resizeIndexBuffer(
			indexBufferId,
			size,
			ManagerWindow::Get()->GetLogicalDeviceId(windowId),
			ManagerWindow::Get()->GetPhysicalDeviceId(windowId));
	}

	VertexBufferId VulkanManagerBuffer::createVertexBuffer(
		const uint64_t size,
		const description::MemoryAccess memoryAccess,
		const VkBufferUsageFlags usage,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId)
	{
		auto nextVertexId = GeneratorId::GenerateUniqueId<VertexBufferId>(next_vertex_id_);

		auto needFind = utils::GuardResize::MayBeResize(
			next_vertex_id_,
			[](const std::shared_ptr<DataVertexBuffer>& val) { return bool(!val); },
			vertex_buffers_,
			detail_vertex_buffer_);

		DetailAttributeBuffer detailAttributeBuffer{};
		detailAttributeBuffer.physical_device_id = physicalDeviceId;
		detailAttributeBuffer.memory_access = memoryAccess;


		detail_vertex_buffer_[nextVertexId] = detailAttributeBuffer;
		vertex_buffers_[nextVertexId] = CreatorVertexBuffer::CreateVertexBuffer(
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			ManagerDevice::Get()->GetCommandPool(logicalDeviceId),
			ManagerDevice::Get()->GetPhysicalDevice(physicalDeviceId),
			ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId),
			usage,
			size,
			memoryAccess);

		return nextVertexId;
	}

	void VulkanManagerBuffer::writeToVertexBuffer(
		const VertexBufferId vertexBufferId,
		const void* data,
		const uint64_t size,
		const uint64_t offset,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId)
	{
		CreatorVertexBuffer::WriteToVertexBuffer(
			GetVertexBuffer(vertexBufferId),
			data,
			size,
			offset,
			detail_vertex_buffer_[vertexBufferId].memory_access,
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			ManagerDevice::Get()->GetCommandPool(logicalDeviceId),
			ManagerDevice::Get()->GetPhysicalDevice(physicalDeviceId),
			ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId));
	}

	void VulkanManagerBuffer::resizeVertexBuffer(
		const VertexBufferId vertexBufferId,
		const uint64_t size,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId)
	{
		auto& vertexBuffer = vertex_buffers_[vertexBufferId];

		const auto& logicalDevice = ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId);
		const auto& commandPool = ManagerDevice::Get()->GetCommandPool(logicalDeviceId);
		const auto& physicalDevice = ManagerDevice::Get()->GetPhysicalDevice(physicalDeviceId);
		const auto& graphicsQueue = ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId);

		auto newVertexBuffer = CreatorVertexBuffer::CreateVertexBuffer(
			logicalDevice,
			commandPool,
			physicalDevice,
			graphicsQueue,
			0,
			size,
			description::MemoryAccess::GPU);

		CreatorBuffer::CopyBuffer(
			logicalDevice->device,
			commandPool->command_pool,
			graphicsQueue,
			vertexBuffer->buf_data,
			newVertexBuffer->buf_data,
			std::min(size, vertexBuffer->buf_data->size),
			0,
			0);

		vertexBuffer->buf_data = newVertexBuffer->buf_data;
		vertexBuffer->device_memory = newVertexBuffer->device_memory;
	}

	IndexBufferId VulkanManagerBuffer::createIndexBuffer(
		const uint64_t size,
		const description::IndexType type,
		const VkBufferUsageFlags usage,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId)
	{
		IndexBufferId curId = GeneratorId::GenerateUniqueId<IndexBufferId>(next_index_id_);

		auto needFind = utils::GuardResize::MayBeResize(
			next_index_id_,
			[](const std::shared_ptr<DataIndexBuffer>& val) { return bool(!val); },
			index_buffers_,
			detail_index_buffer_);

		detail_index_buffer_[curId] = { logicalDeviceId, physicalDeviceId, description::MemoryAccess::GPU };
		index_buffers_[curId] = CreatorIndexBuffer::CreateIndexBuffer(
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			ManagerDevice::Get()->GetCommandPool(logicalDeviceId),
			ManagerDevice::Get()->GetPhysicalDevice(physicalDeviceId),
			ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId),
			usage,
			size,
			type);

		return curId;
	}

	void VulkanManagerBuffer::writeToIndexBuffer(
		const IndexBufferId indexBufferId,
		const void* data,
		const uint64_t size,
		const uint64_t offset,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId)
	{
		CreatorIndexBuffer::WriteToIndexBuffer(
			GetIndexBuffer(indexBufferId),
			data,
			size,
			offset,
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			ManagerDevice::Get()->GetCommandPool(logicalDeviceId),
			ManagerDevice::Get()->GetPhysicalDevice(physicalDeviceId),
			ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId));
	}

	void VulkanManagerBuffer::resizeIndexBuffer(
		const IndexBufferId indexBufferId,
		const uint64_t size,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId)
	{
		auto& indexBuffer = index_buffers_[indexBufferId];

		const auto& logicalDevice = ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId);
		const auto& commandPool = ManagerDevice::Get()->GetCommandPool(logicalDeviceId);
		const auto& physicalDevice = ManagerDevice::Get()->GetPhysicalDevice(physicalDeviceId);
		const auto& graphicsQueue = ManagerDevice::Get()->GetGraphicsQueue(logicalDeviceId);

		auto newIndexBuffer = CreatorIndexBuffer::CreateIndexBuffer(
			logicalDevice,
			commandPool,
			physicalDevice,
			graphicsQueue,
			0,
			size,
			indexBuffer->index_type);

		CreatorBuffer::CopyBuffer(
			logicalDevice->device,
			commandPool->command_pool,
			graphicsQueue,
			indexBuffer->buf_data,
			newIndexBuffer->buf_data,
			std::min(size, indexBuffer->buf_data->size),
			0,
			0);

		indexBuffer->buf_data = newIndexBuffer->buf_data;
		indexBuffer->device_memory = newIndexBuffer->device_memory;
		indexBuffer->num_index = newIndexBuffer->num_index;
	}

	std::shared_ptr<DataVertexBuffer> VulkanManagerBuffer::GetVertexBuffer(const VertexBufferId& id)
	{
		return vertex_buffers_[id];
	}

	std::shared_ptr<DataIndexBuffer> VulkanManagerBuffer::GetIndexBuffer(const IndexBufferId& id)
	{
		return index_buffers_[id];
	}

	void VulkanManagerBuffer::DeleteVertexBuffers(const GraphicsWindowId& windowId, const std::vector<VertexBufferId>& ids)
	{
		auto deleteVertexBuffers = [ids = ids, this]()
		{
			for (auto& id : ids)
			{
				next_vertex_id_ = std::min(next_vertex_id_, id.GetId());
				vertex_buffers_[id] = nullptr;
				detail_vertex_buffer_[id] = DetailAttributeBuffer();
			}
		};

		size_t counter = 1;
		if (!deleted_windows_.contains(windowId))
		{
			const SwapchainId& swapchainId = ManagerWindow::Get()->GetSwapchainId(windowId);
			counter = ManagerSwapchain::Get()->GetNumImages(swapchainId);
		}

		delete_buffers_commands_.emplace(std::make_pair(windowId, std::make_pair(counter, deleteVertexBuffers)));
	}

	void VulkanManagerBuffer::DeleteIndexBuffers(const GraphicsWindowId& windowId, const std::vector<IndexBufferId>& ids)
	{
		auto deleteIndexBuffers = [ids = ids, this]()
		{
			for (auto& id : ids)
			{
				next_index_id_ = std::min(next_index_id_, id.GetId());
				index_buffers_[id] = nullptr;
				detail_index_buffer_[id] = DetailAttributeBuffer();
			}
		};

		size_t counter = 1;
		if (!deleted_windows_.contains(windowId))
		{
			const SwapchainId& swapchainId = ManagerWindow::Get()->GetSwapchainId(windowId);
			counter = ManagerSwapchain::Get()->GetNumImages(swapchainId);
		}

		delete_buffers_commands_.emplace(std::make_pair(windowId, std::make_pair(counter, deleteIndexBuffers)));
	}

	VertexBufferId VulkanManagerBuffer::CreateCopyVertexBuffer(const GraphicsWindowId& windowId, const VertexBufferId& idSrcBuffer)
	{
		VertexBufferId newId = GeneratorId::GenerateUniqueId<VertexBufferId>(next_vertex_id_);

		auto needFind = utils::GuardResize::MayBeResize(
			next_vertex_id_,
			[](const std::shared_ptr<DataVertexBuffer>& val) { return bool(!val); },
			vertex_buffers_,
			detail_vertex_buffer_);

		detail_vertex_buffer_[newId] = detail_vertex_buffer_[idSrcBuffer];
		vertex_buffers_[newId] = vertex_buffers_[idSrcBuffer];

		return newId;
	}

	IndexBufferId VulkanManagerBuffer::CreateCopyIndexBuffer(const GraphicsWindowId& windowId, const IndexBufferId& idSrcBuffer)
	{
		IndexBufferId newId = GeneratorId::GenerateUniqueId<IndexBufferId>(next_index_id_);

		auto needFind = utils::GuardResize::MayBeResize(
			next_index_id_,
			[](const std::shared_ptr<DataIndexBuffer>& val) { return bool(!val); },
			index_buffers_,
			detail_index_buffer_);

		detail_index_buffer_[newId] = detail_index_buffer_[idSrcBuffer];
		index_buffers_[newId] = index_buffers_[idSrcBuffer];

		return newId;
	}

	size_t VulkanManagerBuffer::GetVertexBufferSize(const VertexBufferId& idBuffer)
	{
		return vertex_buffers_[idBuffer]->buf_data->size;
	}

	size_t VulkanManagerBuffer::GetIndexBufferSize(const IndexBufferId& idBuffer)
	{
		return index_buffers_[idBuffer]->buf_data->size;
	}

	void VulkanManagerBuffer::ProcessDeletingBuffers(const GraphicsWindowId& windowId)
	{
		auto [begin, end] = delete_buffers_commands_.equal_range(windowId);

		for (auto iter = begin; iter != end;)
		{
			auto& counter = iter->second.first;
			if (--counter == 0)
			{
				auto& delete_buffer_function = iter->second.second;
				delete_buffer_function();
				iter = delete_buffers_commands_.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}

	void VulkanManagerBuffer::DeletingBuffersForDeletedWindows()
	{
		for (const auto& windowId : deleted_windows_)
		{
			ProcessDeletingBuffers(windowId);
		}

		deleted_windows_.clear();
	}

	void VulkanManagerBuffer::AddDeletedWindow(const GraphicsWindowId& windowId)
	{
		deleted_windows_.insert(windowId);
	}
}
