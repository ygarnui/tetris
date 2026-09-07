#pragma once

#include "../struct_data.h"
#include "../manager_device.h"

#include "manager_buffer.h"
#include "../src/manager_window.h"
#include "../manager_base.h"

#include <buffer_description.h>
#include <generator_id.h>
#include <vector.h>

#include <functional>
#include <map>
#include <set>

namespace render
{
struct DetailAttributeBuffer
{
	RenderId logical_device_id = GeneratorId::GenerateInvalidId();
	// unused, only for info
	RenderId physical_device_id = GeneratorId::GenerateInvalidId();

	description::MemoryAccess memory_access;
};

class VulkanManagerBuffer : public ManagerBuffer, public ManagerBase
{
public:
	static std::shared_ptr<VulkanManagerBuffer>& Get();

	~VulkanManagerBuffer();

	VulkanManagerBuffer(const VulkanManagerBuffer&) = delete;
	VulkanManagerBuffer(VulkanManagerBuffer&&) = delete;

	VulkanManagerBuffer& operator= (const VulkanManagerBuffer&) = delete;
	VulkanManagerBuffer& operator= (VulkanManagerBuffer&&) = delete;

	[[nodiscard]] VertexBufferId CreateVertexBuffer(
		const GraphicsWindowId& windowId,
		const uint64_t size,
		const description::MemoryAccess memoryAccess) override;

	void WriteToVertexBuffer(
		const VertexBufferId& vertexBufferId,
		const GraphicsWindowId& windowId,
		const void* data,
		const uint64_t size, const uint64_t offset) override;

	void ResizeVertexBuffer(
		const VertexBufferId& vertexBufferId,
		const GraphicsWindowId& windowId,
		const uint64_t size) override;

	[[nodiscard]] IndexBufferId CreateIndexBuffer(
		const GraphicsWindowId& windowId,
		const uint64_t size,
		const description::IndexType type) override;

	void WriteToIndexBuffer(
		const IndexBufferId& indexBufferId,
		const GraphicsWindowId& windowId,
		const void* data,
		const uint64_t size,
		const uint64_t offset) override;

	void ResizeIndexBuffer(
		const IndexBufferId& indexBufferId,
		const GraphicsWindowId& windowId,
		const uint64_t size) override;


	[[nodiscard]] std::shared_ptr<DataVertexBuffer> GetVertexBuffer(const VertexBufferId& id) override;
	[[nodiscard]] std::shared_ptr<DataIndexBuffer> GetIndexBuffer(const IndexBufferId& id) override;

	void DeleteVertexBuffers(const GraphicsWindowId& windowId, const std::vector<VertexBufferId>& ids) override;
	void DeleteIndexBuffers(const GraphicsWindowId& windowId, const std::vector<IndexBufferId>& ids) override;

	VertexBufferId CreateCopyVertexBuffer(const GraphicsWindowId& windowId, const VertexBufferId& idSrcBuffer) override;
	IndexBufferId CreateCopyIndexBuffer(const GraphicsWindowId& windowId, const IndexBufferId& idSrcBuffer) override;

	size_t GetVertexBufferSize(const VertexBufferId& idBuffer) override;
	size_t GetIndexBufferSize(const IndexBufferId& idBuffer) override;

	void ProcessDeletingBuffers(const GraphicsWindowId& windowId);
	void DeletingBuffersForDeletedWindows();
	void AddDeletedWindow(const GraphicsWindowId& windowId);

private:
	[[nodiscard]] VertexBufferId createVertexBuffer(
		const uint64_t size,
		const description::MemoryAccess memoryAccess,
		const VkBufferUsageFlags usage,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId);

	void writeToVertexBuffer(
		const VertexBufferId vertexBufferId,
		const void* data,
		const uint64_t size,
		const uint64_t offset,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId);

	void resizeVertexBuffer(
		const VertexBufferId vertexBufferId,
		const uint64_t size,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId);

	[[nodiscard]] IndexBufferId createIndexBuffer(
		const uint64_t size,
		const description::IndexType type,
		const VkBufferUsageFlags usage,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId);

	void writeToIndexBuffer(
		const IndexBufferId vertexBufferId,
		const void* data,
		const uint64_t size,
		const uint64_t offset,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId);

	void resizeIndexBuffer(
		const IndexBufferId indexBufferId,
		const uint64_t size,
		const LogicalDeviceId& logicalDeviceId,
		const PhysicalDeviceId& physicalDeviceId);

	VulkanManagerBuffer();

	size_t next_vertex_id_ = 0;
	size_t next_index_id_ = 0;
	Vector<std::shared_ptr<DataVertexBuffer>, VertexBufferId> vertex_buffers_;
	Vector<std::shared_ptr<DataIndexBuffer>, IndexBufferId> index_buffers_;

	Vector<DetailAttributeBuffer, VertexBufferId> detail_vertex_buffer_;
	Vector<DetailAttributeBuffer, IndexBufferId> detail_index_buffer_;

	std::multimap<GraphicsWindowId, std::pair<size_t, std::function<void()>>> delete_buffers_commands_;
	std::set<GraphicsWindowId> deleted_windows_;
};
}
