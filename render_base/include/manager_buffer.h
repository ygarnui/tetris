#pragma once

#include <buffer_description.h>

#include "graphics_id.h"
#include "../../vulkan_renderer/src/manager_device.h"
#include "../../vulkan_renderer/src/struct_data.h"

namespace render
{
	class ManagerBuffer
	{

	public:
		[[nodiscard]] virtual VertexBufferId CreateVertexBuffer(
			const GraphicsWindowId& windowId,
			const uint64_t size,
			const description::MemoryAccess memoryAccess) = 0;

		virtual void WriteToVertexBuffer(
			const VertexBufferId& vertexBufferId,
			const GraphicsWindowId& windowId,
			const void* data,
			const uint64_t size,
			const uint64_t offset) = 0;

		virtual void ResizeVertexBuffer(
			const VertexBufferId& vertexBufferId,
			const GraphicsWindowId& windowId,
			const uint64_t size) = 0;

		[[nodiscard]] virtual IndexBufferId CreateIndexBuffer(
			const GraphicsWindowId& windowId,
			const uint64_t size,
			const description::IndexType type) = 0;

		virtual void WriteToIndexBuffer(
			const IndexBufferId& indexBufferId,
			const GraphicsWindowId& windowId,
			const void* data,
			const uint64_t size,
			const uint64_t offset) = 0;

		virtual void ResizeIndexBuffer(
			const IndexBufferId& indexBufferId,
			const GraphicsWindowId& windowId,
			const uint64_t size) = 0;

		virtual std::shared_ptr<DataVertexBuffer> GetVertexBuffer(const VertexBufferId& id) = 0;
		virtual std::shared_ptr<DataIndexBuffer> GetIndexBuffer(const IndexBufferId& id) = 0;

		virtual void DeleteVertexBuffers(const GraphicsWindowId& windowId, const std::vector<VertexBufferId>& ids) = 0;
		virtual void DeleteIndexBuffers(const GraphicsWindowId& windowId, const std::vector<IndexBufferId>& ids) = 0;

		virtual VertexBufferId CreateCopyVertexBuffer(const GraphicsWindowId& windowId, const VertexBufferId& idSrcBuffer) = 0;
		virtual IndexBufferId CreateCopyIndexBuffer(const GraphicsWindowId& windowId, const IndexBufferId& idSrcBuffer) = 0;

		virtual size_t GetVertexBufferSize(const VertexBufferId& idBuffer) = 0;
		virtual size_t GetIndexBufferSize(const IndexBufferId& idBuffer) = 0;
	};
}
