#pragma once

#include "graphics_id.h"
#include "generator_id.h"
#include "viewport.h"

#include <glm/glm.hpp>
#include <logger_instance.h>

#include <vector>
#include <optional>
#include <functional>

namespace render
{
	enum class TypeCmdCall
	{
		BeginRenderPass,
		EndRenderPass,
		SetViewport,
		SetScissor,
		SetLineWidth,
		BindPipeline,
		BindIndexBuffer,
		BindVertexBuffers,
		BindDescriptorSets,
		Draw,
		DrawIndexed,
		DrawInderect,
		Dispatch,
		TraceRays,
		ClearColorImage,
		Test,
		Total
	};

	class CmdBaseAbstact
	{
	public:
		CmdBaseAbstact(TypeCmdCall type)
		{
			type_ = type;
		}
		virtual ~CmdBaseAbstact() {}

		TypeCmdCall GetType() const
		{
			return type_;
		}

	private:
		TypeCmdCall type_ = TypeCmdCall::Total;
	};

	class CmdBeginRenderPass : public CmdBaseAbstact
	{
	public:
		CmdBeginRenderPass(const RenderPassId& idRenderPass, const FrameBufferId& idFrameBuffer)
			: CmdBaseAbstact(TypeCmdCall::BeginRenderPass)
		{
			id_render_pass_ = idRenderPass;
			id_frame_buffer_ = idFrameBuffer;

			if (!id_render_pass_.IsValid())
			{
				LOGEXC(std::invalid_argument, "[CmdBeginRenderPass::CheckValid] invalid id_render_pass_");
			}
			if (!id_frame_buffer_.IsValid())
			{
				LOGEXC(std::invalid_argument, "[CmdBeginRenderPass::CheckValid] invalid id_frame_buffer_");
			}
		}
		virtual ~CmdBeginRenderPass() {}

		const RenderPassId& GetRenderPassId() const
		{
			return id_render_pass_;
		}

		const FrameBufferId& GetFrameBufferId() const
		{
			return id_frame_buffer_;
		}

	private:
		RenderPassId id_render_pass_;
		FrameBufferId id_frame_buffer_;
	};

	class CmdEndRenderPass : public CmdBaseAbstact
	{
	public:
		CmdEndRenderPass() : CmdBaseAbstact(TypeCmdCall::EndRenderPass) {}
		virtual ~CmdEndRenderPass() {}
	};

	class CmdSetViewport : public CmdBaseAbstact
	{
	public:
		CmdSetViewport(const std::vector<Viewport>& viewports) : CmdBaseAbstact(TypeCmdCall::SetViewport)
		{
			viewports_ = viewports;

			if (viewports_.size() == 0)
			{
				LOGEXC(std::invalid_argument, "[CmdSetViewport::CmdSetViewport] viewports_ size = 0");
			}
		}

		CmdSetViewport(const FrameBufferId& idFrameBuffer) : CmdBaseAbstact(TypeCmdCall::SetViewport)
		{
			id_frame_buffer_ = idFrameBuffer;

			if (!id_frame_buffer_.IsValid())
			{
				LOGEXC(std::invalid_argument, "[CmdSetViewport::CmdSetViewport] invalid id_frame_buffer_");
			}
		}

		virtual ~CmdSetViewport() {}

		const std::vector<Viewport>& GetViewports() const noexcept
		{
			return viewports_;
		}

		const FrameBufferId& GetFrameBufferId() const noexcept
		{
			return id_frame_buffer_;
		}

		void SetViewports(const std::vector<Viewport>& viewports)
		{
			viewports_ = viewports;
		}

	private:
		std::vector<Viewport> viewports_;
		FrameBufferId id_frame_buffer_;
	};

	class CmdSetScissor : public CmdBaseAbstact
	{
	public:
		struct Rect2D
		{
			glm::i32vec2 offset;
			glm::u32vec2 extent;
		};

	public:
		CmdSetScissor(const std::vector<Rect2D>& scissors): CmdBaseAbstact(TypeCmdCall::SetScissor)
		{
			scissors_ = scissors;

			if (scissors_.size() == 0)
			{
				LOGEXC(std::invalid_argument, "[CmdSetScissor::CmdSetScissor] viewports_ size = 0");
			}
		}

		CmdSetScissor(const FrameBufferId& idFrameBuffer) : CmdBaseAbstact(TypeCmdCall::SetScissor)
		{
			id_frame_buffer_ = idFrameBuffer;

			if (!id_frame_buffer_.IsValid())
			{
				LOGEXC(std::invalid_argument, "[CmdSetScissor::CmdSetScissor] invalid id_frame_buffer_");
			}
		}

		virtual ~CmdSetScissor() {}

		const std::vector<Rect2D>& GetScissors() const
		{
			return scissors_;
		}

		const FrameBufferId& GetFrameBufferId() const
		{
			return id_frame_buffer_;
		}

	private:
		std::vector<Rect2D> scissors_;
		FrameBufferId id_frame_buffer_;
	};

	class CmdSetLineWidth : public CmdBaseAbstact
	{
	public:
		CmdSetLineWidth(const float width) : CmdBaseAbstact(TypeCmdCall::SetLineWidth)
		{
			width_ = width;

			if (width_ <= 0)
			{
				LOGEXC(std::invalid_argument, "[CmdSetLineWidth::CheckValid] width_ <= 0");
			}
		}
		virtual ~CmdSetLineWidth() {}

		float GetWidth() const
		{
			return width_;
		}

	private:
		float width_;
	};

	class CmdBindPipeline : public CmdBaseAbstact
	{
	public:
		CmdBindPipeline(const RenderId& idPipeline) : CmdBaseAbstact(TypeCmdCall::BindPipeline)
		{
			id_pipeline_ = idPipeline;

			if (!id_pipeline_.IsValid())
			{
				LOGEXC(std::invalid_argument, "[CmdBindPipeline::CheckValid] invalid id_pipeline_");
			}
		}
		virtual ~CmdBindPipeline() {}

		const RenderId& GetPipelineId() const
		{
			return id_pipeline_;
		}

	private:
		RenderId id_pipeline_;
	};

	class CmdBindIndexBuffer : public CmdBaseAbstact
	{
	public:
		CmdBindIndexBuffer(const IndexBufferId& idIndexBuffer) : CmdBaseAbstact(TypeCmdCall::BindIndexBuffer)
		{
			id_index_buffer_ = idIndexBuffer;
			
			if (!id_index_buffer_.IsValid())
			{
				LOGEXC(std::invalid_argument, "[CmdBindIndexBuffer::CheckValid] invalid id_index_buffer_");
			}
		}
		virtual ~CmdBindIndexBuffer() {}

		const IndexBufferId& GetIndexBufferId() const
		{
			return id_index_buffer_;
		}

	private:
		IndexBufferId id_index_buffer_;
	};

	class CmdBindVertexBuffers : public CmdBaseAbstact
	{
	public:
		CmdBindVertexBuffers(const std::vector<VertexBufferId>& idVertexBuffers) : CmdBaseAbstact(TypeCmdCall::BindVertexBuffers)
		{
			id_vertex_buffers_ = idVertexBuffers;
			
			for (size_t i = 0; i < id_vertex_buffers_.size(); i++)
			{
				if (!id_vertex_buffers_[i].IsValid())
				{
					LOGEXC(std::invalid_argument, "[CmdBindVertexBuffers::CheckValid] invalid id in id_vertex_buffers_ index=", i);
				}
			}
		}
		virtual ~CmdBindVertexBuffers() {}

		const std::vector<VertexBufferId>& GetVertexBuffersId() const
		{
			return id_vertex_buffers_;
		}

	private:
		std::vector<VertexBufferId> id_vertex_buffers_;
	};

	class CmdBindDescriptorSets : public CmdBaseAbstact
	{
	public:
		CmdBindDescriptorSets(const RenderId& idPipeline, const std::vector<DescriptorSetId>& idDescriptorSets) : CmdBaseAbstact(TypeCmdCall::BindDescriptorSets)
		{
			id_pipeline_ = idPipeline;
			id_descriptor_sets_ = idDescriptorSets;
		
			if (!id_pipeline_.IsValid())
			{
				LOGEXC(std::invalid_argument, "[CmdBindDescriptorSets::CheckValid] invalid id_pipeline_");
			}

			for (size_t i = 0; i < id_descriptor_sets_.size(); i++)
			{
				if (!id_descriptor_sets_[i].IsValid())
				{
					LOGEXC(std::invalid_argument, "[CmdBindDescriptorSets::CheckValid] invalid id in id_descriptor_sets_ index=", i);
				}
			}
		}
		virtual ~CmdBindDescriptorSets() {}

		const RenderId& GetPipelineId() const
		{
			return id_pipeline_;
		}

		const std::vector<DescriptorSetId>& GetDescriptorSetsId() const
		{
			return id_descriptor_sets_;
		}

	private:
		RenderId id_pipeline_;
		std::vector<DescriptorSetId> id_descriptor_sets_;
	};

	class CmdDraw : public CmdBaseAbstact
	{
	public:
		CmdDraw(const uint32_t vertexCount) : CmdBaseAbstact(TypeCmdCall::Draw)
		{
			vertex_count_ = vertexCount;
			
			if (vertex_count_ == 0)
			{
				LOGEXC(std::invalid_argument, "[CmdDraw::CheckValid] vertex_count_ = 0");
			}
		}
		virtual ~CmdDraw() {}

		uint32_t GetVertexCount() const
		{
			return vertex_count_;
		}

	private:
		uint32_t vertex_count_;
	};

	class CmdDrawIndexed : public CmdBaseAbstact
	{
	public:
		CmdDrawIndexed(const IndexBufferId& idIndexBuffer) : CmdBaseAbstact(TypeCmdCall::DrawIndexed)
		{
			id_index_buffer_ = idIndexBuffer;
			
			if (!id_index_buffer_.IsValid())
			{
				LOGEXC(std::invalid_argument, "[CmdDrawIndexed::CheckValid] invalid id_index_buffer_");
			}
		}
		virtual ~CmdDrawIndexed() {}

		const IndexBufferId& GetIndexBufferId() const
		{
			return id_index_buffer_;
		}

	private:
		IndexBufferId id_index_buffer_;
	};

	class CmdDrawInderect : public CmdBaseAbstact
	{
	public:
		CmdDrawInderect(std::vector<void*> ids) : CmdBaseAbstact(TypeCmdCall::DrawInderect)
		{
			buffer_id = ids;
		}
		virtual ~CmdDrawInderect() {}

		std::vector<void*> buffer_id;

	private:
	};

	class CmdDispatch : public CmdBaseAbstact
	{
	public:
		CmdDispatch(const uint32_t x, const uint32_t y, const uint32_t z) : CmdBaseAbstact(TypeCmdCall::Dispatch)
		{
			x_ = x;
			y_ = y;
			z_ = z;
			
			if (x_ == 0)
			{
				LOGEXC(std::invalid_argument, "[CmdDispatch::CheckValid]  x_ = 0");
			}
			if (y_ == 0)
			{
				LOGEXC(std::invalid_argument, "[CmdDispatch::CheckValid]  y_ = 0");
			}
			if (z_ == 0)
			{
				LOGEXC(std::invalid_argument, "[CmdDispatch::CheckValid]  z_ = 0");
			}
		}
		virtual ~CmdDispatch() {}

		uint32_t GetX() const
		{
			return x_;
		}

		uint32_t GetY() const
		{
			return y_;
		}

		uint32_t GetZ() const
		{
			return z_;
		}

	private:
		uint32_t x_;
		uint32_t y_;
		uint32_t z_;
	};


	/*!
	\brief Trace the whole frame of a window.

	The window owns exactly one ray tracing pass, so this single command expands into the
	layout transitions, the pipeline and descriptor binding, and the traceRays call that
	produce the frame. The ray generation shader writes straight into the swapchain image,
	which is why no render pass surrounds this command.
	*/
	class CmdTraceRays : public CmdBaseAbstact
	{
	public:
		CmdTraceRays(const GraphicsWindowId& idWindow) : CmdBaseAbstact(TypeCmdCall::TraceRays)
		{
			id_window_ = idWindow;

			if (!id_window_.IsValid())
			{
				LOGEXC(std::invalid_argument, "[CmdTraceRays::CmdTraceRays] invalid id_window_");
			}
		}
		virtual ~CmdTraceRays() {}

		const GraphicsWindowId& GetWindowId() const
		{
			return id_window_;
		}

	private:
		GraphicsWindowId id_window_;
	};

	class CmdClearColorImage : public CmdBaseAbstact
	{
	public:
		CmdClearColorImage(const std::vector<TextureId>& idTextures) : CmdBaseAbstact(TypeCmdCall::ClearColorImage)
		{
			id_textures = idTextures;
			
			for (size_t i = 0; i < id_textures.size(); i++)
			{
				if (!id_textures[i].IsValid())
				{
					LOGEXC(std::invalid_argument, "[CmdClearColorImage::CheckValid] invalid id in id_textures index=", i);
				}
			}
		}
		virtual ~CmdClearColorImage() {}

		const std::vector<TextureId>& GetTexturesId() const
		{
			return id_textures;
		}

	private:
		std::vector<TextureId> id_textures;
	};


	class CmdTest : public CmdBaseAbstact
	{
	public:
		CmdTest(std::function<void(size_t num)> func) : CmdBaseAbstact(TypeCmdCall::Test)
		{
			func_ = func;
		}
		virtual ~CmdTest() {}

		std::function<void(size_t num)> func_;
	private:
	};
}
