#pragma once

#include "creator_acceleration_structure.h"
#include "../struct_data.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace render
{
	/*!
	\brief The shader binding table of a ray tracing pipeline.

	Holds the group handles the driver dispatches through, plus the strided regions
	vkCmdTraceRaysKHR is called with.
	*/
	struct DataShaderBindingTable
	{
		BufferWithMemory buffer;

		VkStridedDeviceAddressRegionKHR raygen_region{};
		VkStridedDeviceAddressRegionKHR miss_region{};
		VkStridedDeviceAddressRegionKHR hit_region{};
		VkStridedDeviceAddressRegionKHR callable_region{};
	};

	class CreatorRayTracingPipeline
	{
	public:
		/*!
		\brief Create a ray tracing pipeline from a set of shader stages.

		The stages are sorted into shader groups by their stage bit, so the caller does not
		have to pass them in any particular order. Exactly one raygen stage is required;
		miss and closest hit stages are optional but a scene without them renders nothing.
		\param[in] device the logical device
		\param[in] pipelineLayout the layout the pipeline is created with
		\param[in] stages the compiled shader stages
		\param[in] maxRecursionDepth how deep traceRayEXT may nest; 1 means rays are only
		traced from the ray generation shader
		\return the created pipeline
		\throw runtime_error if there is no raygen stage or the pipeline cannot be created
		*/
		[[nodiscard]] static std::shared_ptr<DataPipeline> CreatePipeline(
			std::shared_ptr<DataDevice> device,
			std::shared_ptr<DataPipelineLayout> pipelineLayout,
			const std::vector<VkPipelineShaderStageCreateInfo>& stages,
			const uint32_t maxRecursionDepth);

		/*!
		\brief Build the shader binding table of a pipeline created by CreatePipeline.

		Must be called with the same stages, so that the group layout matches the one the
		pipeline was created with.
		\param[in] context the device and physical device the table is allocated on
		\param[in] pipeline the ray tracing pipeline whose group handles are queried
		\param[in] stages the same stages CreatePipeline was given
		\return the filled table with its regions ready for vkCmdTraceRaysKHR
		\throw runtime_error if the group handles cannot be queried
		*/
		[[nodiscard]] static DataShaderBindingTable CreateShaderBindingTable(
			const BuildContext& context,
			std::shared_ptr<DataPipeline> pipeline,
			const std::vector<VkPipelineShaderStageCreateInfo>& stages);

		/*!
		\brief Query the ray tracing pipeline properties of a physical device.
		\param[in] physicalDevice the device to query
		\return the properties, including shader group handle size and alignments
		*/
		[[nodiscard]] static VkPhysicalDeviceRayTracingPipelinePropertiesKHR GetPipelineProperties(
			VkPhysicalDevice physicalDevice);

	private:
		/*!
		\brief Shader group layout derived from a set of stages.
		*/
		struct GroupLayout
		{
			std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;
			uint32_t raygen_count = 0;
			uint32_t miss_count = 0;
			uint32_t hit_count = 0;
		};

		[[nodiscard]] static GroupLayout buildGroupLayout(const std::vector<VkPipelineShaderStageCreateInfo>& stages);
	};
}
