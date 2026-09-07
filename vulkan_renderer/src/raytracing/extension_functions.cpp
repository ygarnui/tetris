#include "extension_functions.h"

#include "../struct_data.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{

PFN_vkCreateAccelerationStructureKHR ExtensionFunctions::CreateAccelerationStructure = nullptr;
PFN_vkDestroyAccelerationStructureKHR ExtensionFunctions::DestroyAccelerationStructure = nullptr;
PFN_vkGetAccelerationStructureBuildSizesKHR ExtensionFunctions::GetAccelerationStructureBuildSizes = nullptr;
PFN_vkGetAccelerationStructureDeviceAddressKHR ExtensionFunctions::GetAccelerationStructureDeviceAddress = nullptr;
PFN_vkCmdBuildAccelerationStructuresKHR ExtensionFunctions::CmdBuildAccelerationStructures = nullptr;

PFN_vkCreateRayTracingPipelinesKHR ExtensionFunctions::CreateRayTracingPipelines = nullptr;
PFN_vkGetRayTracingShaderGroupHandlesKHR ExtensionFunctions::GetRayTracingShaderGroupHandles = nullptr;
PFN_vkCmdTraceRaysKHR ExtensionFunctions::CmdTraceRays = nullptr;

namespace
{
	template<typename T>
	void resolve(T& function, VkDevice device, const char* name)
	{
		function = reinterpret_cast<T>(vkGetDeviceProcAddr(device, name));
		if (!function)
		{
			LOGEXC(std::runtime_error, "[ExtensionFunctions::Load] failed to resolve entry point:", name);
		}
	}
}

void ExtensionFunctions::Load(std::shared_ptr<DataDevice> device)
{
	if (!device)
	{
		LOGEXC(std::invalid_argument, "[ExtensionFunctions::Load] device is null");
	}

	VkDevice logicalDevice = device->device;

	resolve(CreateAccelerationStructure, logicalDevice, "vkCreateAccelerationStructureKHR");
	resolve(DestroyAccelerationStructure, logicalDevice, "vkDestroyAccelerationStructureKHR");
	resolve(GetAccelerationStructureBuildSizes, logicalDevice, "vkGetAccelerationStructureBuildSizesKHR");
	resolve(GetAccelerationStructureDeviceAddress, logicalDevice, "vkGetAccelerationStructureDeviceAddressKHR");
	resolve(CmdBuildAccelerationStructures, logicalDevice, "vkCmdBuildAccelerationStructuresKHR");

	resolve(CreateRayTracingPipelines, logicalDevice, "vkCreateRayTracingPipelinesKHR");
	resolve(GetRayTracingShaderGroupHandles, logicalDevice, "vkGetRayTracingShaderGroupHandlesKHR");
	resolve(CmdTraceRays, logicalDevice, "vkCmdTraceRaysKHR");

	is_loaded_ = true;

	LOG(Loglvl::info, "[ExtensionFunctions::Load] ray tracing entry points resolved");
}

bool ExtensionFunctions::IsLoaded() noexcept
{
	return is_loaded_;
}

}
