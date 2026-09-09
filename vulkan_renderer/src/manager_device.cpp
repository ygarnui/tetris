#include "manager_device.h"

#include "creator_physical_device.h"
#include "creator_queue_descriptor.h"
#include "creator_command_pool.h"
#include "creator_logical_device.h"
#include "creator_swapchain_support_details.h"
#include "filter_suitable_devices.h"
#include "struct_base_data.h"

#include <logger_instance.h>
#include <generator_id.h>

#include <stdexcept>

namespace render
{
	ManagerDevice::ManagerDevice()
	{
	}

	std::shared_ptr<ManagerDevice>& ManagerDevice::Get()
	{
		static std::shared_ptr<ManagerDevice> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<ManagerDevice>(new ManagerDevice());
		}
		return manager;
	}

	ManagerDevice::~ManagerDevice()
	{
		LOG(Loglvl::debug, "[ManagerDevice::~ManagerDevice]");
	}

	void ManagerDevice::Init(std::shared_ptr<DataInstance> instance, const std::vector<const char*>& requiredExtensions)
	{
		instance_ = instance;

		physical_devices_ = CreatorPhysicalDevice::CreatePhysicalDevices(instance);

		if (physical_devices_.empty())
		{
			LOGEXC(std::runtime_error, "[ManagerDevice::Init] No suitable graphics card!");
			return;
		}

		bool foundPriorityDevice = false;

		for (size_t i = 0; i < physical_devices_.size(); i++)
		{
			const auto id = GeneratorId::GenerateUniqueId<PhysicalDeviceId>(i);
			PhysicalDeviceDetails& physicalDeviceDetails = physical_device_details_[id];
			vkGetPhysicalDeviceProperties(physical_devices_[id], &physicalDeviceDetails.vk_properties);
			CreatorPhysicalDevice::PrintInfo(physicalDeviceDetails.vk_properties);

			if (foundPriorityDevice)
			{
				continue;
			}

			// No window/surface exists yet at this point, so this only checks what can be known
			// without one: device extensions (ray tracing included) and a graphics queue family.
			// AddWindow re-checks the chosen device against the real surface later via
			// IsPhysicalDeviceSuitable, once one exists.
			PhysicalDeviceProperties properties{};
			const bool suitable = FilterSuitableDevices::CheckPhysicalDevice(
				physical_devices_[id],
				nullptr,
				requiredExtensions,
				DataSwapchainSupportDetails{},
				properties);

			if (suitable)
			{
				priority_device_id_ = id;
				foundPriorityDevice = true;
			}
		}

		if (!foundPriorityDevice)
		{
			LOGEXC(std::runtime_error, "[ManagerDevice::Init] No physical device supports the required extensions (ray tracing included)!");
		}
	}

	size_t ManagerDevice::GetCountPhysicalDevice() const noexcept
	{
		return physical_devices_.size();
	}

	VkPhysicalDevice ManagerDevice::GetPhysicalDevice(const PhysicalDeviceId& physicalDeviceId) const
	{
		return physical_devices_[physicalDeviceId];
	}

	LogicalDeviceId ManagerDevice::AddLogicalDevice(
		const PhysicalDeviceId& physicalDeviceId)
	{
		auto logicalDeviceId = GeneratorId::GenerateUniqueId<LogicalDeviceId>(logical_device_.size());

		physical_ids_by_logical_id[logicalDeviceId] = physicalDeviceId;
		logical_ids_by_physical_id[physicalDeviceId] = logicalDeviceId;

		PhysicalDeviceDetails physicalDeviceDetails = physical_device_details_[physicalDeviceId];

		logical_device_.push_back(CreatorLogicalDevice::CreateLogicalDevice(
			physical_devices_[physicalDeviceId],
			physicalDeviceDetails.properties_.queue_family_indices,
			physicalDeviceDetails.properties_.device_features,
			physicalDeviceDetails.extensions_,
			instance_));

		graphics_queue_.push_back(CreatorQueueDescription::CreateQueueDescription(
			logical_device_[logicalDeviceId],
			physicalDeviceDetails.properties_.queue_family_indices));

		command_pool_.push_back(CreatorCommandPool::CreateCommandPool(
			logical_device_[logicalDeviceId],
			physicalDeviceDetails.properties_.queue_family_indices[0]));

		return logicalDeviceId;
	}

	std::shared_ptr<DataDevice> ManagerDevice::GetLogicalDevice(const LogicalDeviceId& logicalDeviceId)
	{
		return logical_device_[logicalDeviceId];
	}

	PhysicalDeviceId ManagerDevice::GetPhysicalDeviceIdByLogicalDeviceId(const LogicalDeviceId& logicalDeviceId) const
	{
		return physical_ids_by_logical_id.at(logicalDeviceId);
	}

	LogicalDeviceId ManagerDevice::GetLogicalDeviceIdByPhysicalDeviceId(const PhysicalDeviceId& physicalDeviceId) const
	{
		auto logicalDevice = logical_ids_by_physical_id.find(physicalDeviceId);
		if (logicalDevice != logical_ids_by_physical_id.end())
		{
			return logicalDevice->second;
		}

		return GeneratorId::GenerateInvalidId<LogicalDeviceId>();
	}

	PhysicalDeviceProperties ManagerDevice::GetPhysicalDeviceProperties(const PhysicalDeviceId& physicalDeviceId) const
	{
		return physical_device_details_.at(physicalDeviceId).properties_;
	}

	const VkPhysicalDeviceProperties& ManagerDevice::GetVkPhysicalDeviceProperties(const PhysicalDeviceId& physicalDeviceId) const
	{
		return physical_device_details_.at(physicalDeviceId).vk_properties;
	}

	VkQueue ManagerDevice::GetGraphicsQueue(const LogicalDeviceId& logicalDeviceId) const
	{
		return graphics_queue_[logicalDeviceId][0];
	}

	std::shared_ptr<DataCommandPool> ManagerDevice::GetCommandPool(const LogicalDeviceId& logicalDeviceId) const
	{
		return command_pool_[logicalDeviceId];
	}

	PhysicalDeviceId ManagerDevice::GetPriorityPhysicalDeviceId() const
	{
		return priority_device_id_;
	}

	const DataSwapchainSupportDetails& ManagerDevice::GetSwapchainSupportDetailsData(
		const PhysicalDeviceId& physicalDeviceId,
		const SurfaceId& surfaceId)
	{
		return physical_device_details_.at(physicalDeviceId).swapchain_support_details_[surfaceId];
	}

	bool ManagerDevice::IsPhysicalDeviceSuitable(
		const PhysicalDeviceId& physicalDeviceId,
		const SurfaceId& surfaceId,
		std::shared_ptr<DataSurface> surface,
		const std::vector<const char*>& extensions)
	{
		PhysicalDeviceDetails& physicalDeviceDetails = physical_device_details_.at(physicalDeviceId);

		DataSwapchainSupportDetails swapchainSupportDetails{};
		if (surface)
		{
			AddSurfaceCapabilities(physicalDeviceId, surfaceId, surface);
			swapchainSupportDetails = GetSwapchainSupportDetailsData(physicalDeviceId, surfaceId);
		}

		PhysicalDeviceProperties properties{};
		bool isSuitable = FilterSuitableDevices::CheckPhysicalDevice(
			GetPhysicalDevice(physicalDeviceId),
			surface,
			extensions,
			swapchainSupportDetails,
			properties);

		physicalDeviceDetails.extensions_ = extensions;
		physicalDeviceDetails.properties_ = properties;

		return isSuitable;
	}

	void ManagerDevice::AddSurfaceCapabilities(
		const PhysicalDeviceId& physicalDeviceId,
		const SurfaceId& surfaceId,
		std::shared_ptr<DataSurface> surface)
	{
		physical_device_details_.at(physicalDeviceId).swapchain_support_details_[surfaceId] = 
			CreatorSwapchainSupportDetails::CreateSwapchainSupportDetails(GetPhysicalDevice(physicalDeviceId), surface->surface);
	}

	void ManagerDevice::DeviceWaitIdle()
	{
		for (auto device : logical_device_)
		{
			if (!device)
			{
				continue;
			}
			vkDeviceWaitIdle(device->device);
		}
	}
}
