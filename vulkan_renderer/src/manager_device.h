#pragma once

#include "struct_data.h"
#include "device_property.h"
#include "manager_base.h"

#include <graphics_id.h>

#include <map>
#include <vector>

namespace render
{
	class ManagerDevice : public ManagerBase
	{
	public:
		static std::shared_ptr<ManagerDevice>& Get();

		~ManagerDevice();

		/*!
		\brief Enumerate physical devices and pick the priority one.
		\param[in] instance the Vulkan instance to enumerate physical devices from
		\param[in] requiredExtensions device extensions the app cannot run without (ray tracing
		included) - the priority device is the first enumerated physical device that supports
		all of them. Suitability re-checked against the real surface later in
		IsPhysicalDeviceSuitable, once one exists.
		\throw runtime_error if no physical device supports requiredExtensions
		*/
		void Init(std::shared_ptr<DataInstance> instance, const std::vector<const char*>& requiredExtensions);

		ManagerDevice(const ManagerDevice&) = delete;
		ManagerDevice(ManagerDevice&&) = delete;

		ManagerDevice& operator= (const ManagerDevice&) = delete;
		ManagerDevice& operator= (ManagerDevice&&) = delete;

		[[nodiscard]] size_t GetCountPhysicalDevice() const noexcept;

		[[nodiscard]] VkPhysicalDevice GetPhysicalDevice(const PhysicalDeviceId& physicalDeviceId) const;

		[[nodiscard]] LogicalDeviceId AddLogicalDevice(const PhysicalDeviceId& physicalDeviceId);

		[[nodiscard]] std::shared_ptr<DataDevice> GetLogicalDevice(const LogicalDeviceId& logicalDeviceId);

		[[nodiscard]] PhysicalDeviceId GetPhysicalDeviceIdByLogicalDeviceId(const LogicalDeviceId& logicalDeviceId) const;

		[[nodiscard]] LogicalDeviceId GetLogicalDeviceIdByPhysicalDeviceId(const PhysicalDeviceId& physicalDeviceId) const;

		[[nodiscard]] PhysicalDeviceProperties GetPhysicalDeviceProperties(const PhysicalDeviceId& physicalDeviceId) const;

		[[nodiscard]] const VkPhysicalDeviceProperties& GetVkPhysicalDeviceProperties(const PhysicalDeviceId& physicalDeviceId) const;

		[[nodiscard]] VkQueue GetGraphicsQueue(const LogicalDeviceId& logicalDeviceId) const;

		[[nodiscard]] std::shared_ptr<DataCommandPool> GetCommandPool(const LogicalDeviceId& logicalDeviceId) const;

		[[nodiscard]] PhysicalDeviceId GetPriorityPhysicalDeviceId() const;

		[[nodiscard]] const DataSwapchainSupportDetails& GetSwapchainSupportDetailsData(
			const PhysicalDeviceId& physicalDeviceId,
			const SurfaceId& surfaceId);

		/*!
		\brief Checks whether the device is suitable in surface and extensions,
		and fills in the device properties with suitable properties.
		\return true if the device is suitable, otherwise false.
		*/
		bool IsPhysicalDeviceSuitable(
			const PhysicalDeviceId& physicalDeviceId,
			const SurfaceId& surfaceId,
			std::shared_ptr<DataSurface> surface,
			const std::vector<const char*>& extensions);

		void AddSurfaceCapabilities(
			const PhysicalDeviceId& physicalDeviceId,
			const SurfaceId& surfaceId,
			std::shared_ptr<DataSurface> surface);

		void DeviceWaitIdle();

	private:
		ManagerDevice();

		struct PhysicalDeviceDetails
		{
			VkPhysicalDeviceProperties vk_properties;
			PhysicalDeviceProperties properties_;
			std::vector<const char*> extensions_;
			std::map<SurfaceId, DataSwapchainSupportDetails> swapchain_support_details_;
		};

		std::shared_ptr<DataInstance> instance_;

		std::vector<VkPhysicalDevice> physical_devices_;

		std::map<PhysicalDeviceId, PhysicalDeviceDetails> physical_device_details_;
		std::map<LogicalDeviceId, PhysicalDeviceId> physical_ids_by_logical_id;
		std::map<PhysicalDeviceId, LogicalDeviceId> logical_ids_by_physical_id;

		std::vector<std::shared_ptr<DataDevice>> logical_device_;

		PhysicalDeviceId priority_device_id_;

		std::vector<std::vector<VkQueue>> graphics_queue_;

		std::vector<std::shared_ptr<DataCommandPool>> command_pool_;
	};
}
