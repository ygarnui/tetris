#pragma once

#include "creator_acceleration_structure.h"
#include "creator_ray_tracing_pipeline.h"
#include "../manager_base.h"
#include "../struct_data.h"

#include <graphics_id.h>

#include <map>
#include <memory>
#include <vector>

namespace render
{
	/*!
	\brief Descriptor bindings of the ray tracing pass, in set 0.

	This is a fixed contract shared with the shaders: assets/shaders/globals/raytracing.h
	declares the same numbers on the GLSL side.
	*/
	enum class RayTracingBinding : uint32_t
	{
		AccelerationStructure = 0,
		OutputImage = 1,
		Camera = 2,
		Vertices = 3,
		Indices = 4,
		Instances = 5,
	};

	/*!
	\brief Everything the caller has to supply to set up a ray tracing pass.
	*/
	struct RayTracingPassDescription
	{
		GraphicsWindowId window_id;
		ShaderProgramId shader_program_id;

		std::shared_ptr<DataAccelerationStructure> top_level;

		/*! \brief One camera buffer per swapchain image, so a frame in flight is never overwritten. */
		std::vector<std::shared_ptr<DataBuffer>> camera_buffers;

		std::shared_ptr<DataBuffer> vertex_buffer;
		std::shared_ptr<DataBuffer> index_buffer;
		std::shared_ptr<DataBuffer> instance_buffer;

		/*!
		\brief How deep traceRayEXT may nest.

		The shaders here trace every ray, including shadow and reflection rays, from the ray
		generation shader in a loop, so one level is enough regardless of the bounce count.
		*/
		uint32_t max_recursion_depth = 1;
	};

	/*!
	\brief A ray tracing pass ready to be recorded into a command buffer.
	*/
	struct DetailRayTracingPass
	{
		SwapchainId swapchain_id;
		LogicalDeviceId logical_device_id;

		std::shared_ptr<DataPipeline> pipeline;
		std::shared_ptr<DataPipelineLayout> pipeline_layout;
		std::shared_ptr<DataDescriptorPool> descriptor_pool;
		std::shared_ptr<DataDescriptorSetLayout> descriptor_set_layout;

		/*! \brief One set per swapchain image; each points at that image as the output. */
		std::vector<VkDescriptorSet> descriptor_sets;

		DataShaderBindingTable shader_binding_table;

		std::shared_ptr<DataAccelerationStructure> top_level;

		VkExtent2D extent{};
	};

	/*!
	\brief Owns the ray tracing pass of each window.

	One pass per window is enough for this renderer: the whole frame is produced by a single
	traceRays call writing straight into the acquired swapchain image.
	*/
	class ManagerRayTracing : public ManagerBase
	{
	public:
		static std::shared_ptr<ManagerRayTracing>& Get();

		~ManagerRayTracing();

		ManagerRayTracing(const ManagerRayTracing&) = delete;
		ManagerRayTracing(ManagerRayTracing&&) = delete;

		ManagerRayTracing& operator= (const ManagerRayTracing&) = delete;
		ManagerRayTracing& operator= (ManagerRayTracing&&) = delete;

		/*!
		\brief Build the ray tracing pipeline, its shader binding table and its descriptor sets.
		\param[in] description the shader program and the resources to bind
		\throw runtime_error if the pipeline or the descriptors cannot be created
		*/
		void CreatePass(const RayTracingPassDescription& description);

		[[nodiscard]] bool HasPass(const GraphicsWindowId& windowId) const;

		/*!
		\brief Get the pass of a window.
		\throw runtime_error if the window has no pass
		*/
		[[nodiscard]] const DetailRayTracingPass& GetPass(const GraphicsWindowId& windowId) const;

		/*!
		\brief Point the descriptor sets at a newly built top level structure.

		The top level structure is rebuilt whenever the scene moves, which invalidates the
		handle the descriptor sets hold.
		\param[in] windowId the window whose pass is updated
		\param[in] topLevel the freshly built structure
		*/
		void UpdateTopLevel(
			const GraphicsWindowId& windowId,
			std::shared_ptr<DataAccelerationStructure> topLevel);

		void DeletePass(const GraphicsWindowId& windowId);

	private:
		ManagerRayTracing() = default;

		void writeDescriptorSets(
			DetailRayTracingPass& pass,
			const RayTracingPassDescription& description);

		std::map<GraphicsWindowId, DetailRayTracingPass> passes_;
	};
}
