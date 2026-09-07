#pragma once

#include <vulkan/vulkan.h>

#include <memory>

namespace render
{
	struct DataDevice;

	/*!
	\brief Entry points of the ray tracing extensions.

	The Vulkan loader does not export extension entry points statically,
	so they have to be resolved per logical device via vkGetDeviceProcAddr.
	*/
	class ExtensionFunctions
	{
	public:
		/*!
		\brief Resolve every ray tracing entry point for the given device.
		\param[in] device the logical device the ray tracing extensions were enabled on
		\throw runtime_error if any of the entry points is missing
		*/
		static void Load(std::shared_ptr<DataDevice> device);

		[[nodiscard]] static bool IsLoaded() noexcept;

		static PFN_vkCreateAccelerationStructureKHR CreateAccelerationStructure;
		static PFN_vkDestroyAccelerationStructureKHR DestroyAccelerationStructure;
		static PFN_vkGetAccelerationStructureBuildSizesKHR GetAccelerationStructureBuildSizes;
		static PFN_vkGetAccelerationStructureDeviceAddressKHR GetAccelerationStructureDeviceAddress;
		static PFN_vkCmdBuildAccelerationStructuresKHR CmdBuildAccelerationStructures;

		static PFN_vkCreateRayTracingPipelinesKHR CreateRayTracingPipelines;
		static PFN_vkGetRayTracingShaderGroupHandlesKHR GetRayTracingShaderGroupHandles;
		static PFN_vkCmdTraceRaysKHR CmdTraceRays;

	private:
		static inline bool is_loaded_ = false;
	};
}
