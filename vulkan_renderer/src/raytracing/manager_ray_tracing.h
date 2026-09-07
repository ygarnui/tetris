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

		/*!
		\brief Size of the per frame uniform block bound at RayTracingBinding::Camera.

		The pass allocates one buffer of this size per swapchain image itself, so that a
		swapchain recreation which changes the image count cannot leave the caller holding
		the wrong number of buffers. Fill them through WriteUniform.
		*/
		uint64_t uniform_buffer_size = 0;

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
		/*! \brief Kept so the pass can rebuild its descriptors after a swapchain recreation. */
		RayTracingPassDescription description;

		SwapchainId swapchain_id;
		LogicalDeviceId logical_device_id;
		PhysicalDeviceId physical_device_id;

		std::shared_ptr<DataPipeline> pipeline;
		std::shared_ptr<DataPipelineLayout> pipeline_layout;
		std::shared_ptr<DataDescriptorPool> descriptor_pool;
		std::shared_ptr<DataDescriptorSetLayout> descriptor_set_layout;

		/*! \brief One set per swapchain image; each points at that image as the output. */
		std::vector<VkDescriptorSet> descriptor_sets;

		/*! \brief One uniform buffer per swapchain image, matching the sets above. */
		std::vector<BufferWithMemory> uniform_buffers;

		/*! \brief Uniform block of the frame being prepared, until its image index is known. */
		std::vector<uint8_t> staged_uniform;

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
		\brief Hand the pass the uniform block for the next frame.

		The data is kept on the cpu until the frame knows which swapchain image it draws
		into, because the descriptor set of that image is the one that will be read.
		Uploading happens in UploadUniform.
		\param[in] windowId the window whose pass is written to
		\param[in] data source bytes
		\param[in] size number of bytes, must not exceed the size the pass was created with
		\throw runtime_error if the window has no pass or the data does not fit
		*/
		void SetUniform(
			const GraphicsWindowId& windowId,
			const void* data,
			const uint64_t size);

		/*!
		\brief Copy the staged uniform block into the buffer of the acquired swapchain image.

		Called once the image index of the frame is known. Acquiring an image means the
		presentation engine released it, so the previous frame that drew into it has
		finished and its uniform buffer can be overwritten.
		\param[in] swapchainId the swapchain the image was acquired from
		\param[in] imageIndex the acquired image
		*/
		void UploadUniform(const SwapchainId& swapchainId, const uint32_t imageIndex);

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

		/*!
		\brief Rebind a pass to the swapchain images after the swapchain was recreated.

		Both the output extent and the image views held by the descriptors change with the
		swapchain, and the image count may change with it, in which case the descriptor
		resources are rebuilt. Does nothing when no pass uses the given swapchain.

		The caller must have waited for the device to go idle first.
		\param[in] swapchainId the swapchain that was recreated
		*/
		void ApplyResize(const SwapchainId& swapchainId);

		void DeletePass(const GraphicsWindowId& windowId);

	private:
		ManagerRayTracing() = default;

		/*!
		\brief Allocate the descriptor pool, the descriptor sets and one uniform buffer per image.
		*/
		void createDescriptorResources(DetailRayTracingPass& pass, const uint32_t numImages);

		void writeDescriptorSets(DetailRayTracingPass& pass);

		std::map<GraphicsWindowId, DetailRayTracingPass> passes_;
	};
}
