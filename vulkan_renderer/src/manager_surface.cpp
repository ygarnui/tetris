#include "manager_surface.h"
#include "creator_surface_khr.h"
#include "creator_swapchain_support_details.h"
#include "filter_suitable_devices.h"
#include "struct_base_data.h"

#include <logger_instance.h>
#include <generator_id.h>

namespace render
{
	ManagerSurface::ManagerSurface()
	{
	}

	SurfaceId ManagerSurface::AddSurface(
		std::shared_ptr<DataInstance> instance,
		const std::function<DataResult(std::shared_ptr<DataInstance>, std::shared_ptr<DataSurface>)>& createWindowSurface,
		bool externalDeleter)
	{
		auto newId = GeneratorId::GenerateUniqueId<SurfaceId>(surface_.size());
		surface_.push_back(CreatorSurfaceKHR::CreateSurfaceKHR(instance, createWindowSurface, externalDeleter));
		create_window_surface_.push_back(createWindowSurface);

		return newId;
	}

	void ManagerSurface::DeleteSurface(SurfaceId& surfaceId)
	{
		if (!surfaceId.IsValid())
		{
			return;
		}

		surface_[surfaceId] = nullptr;

		surfaceId = GeneratorId::GenerateUniqueId<SurfaceId>(-1);
	}

	std::shared_ptr<ManagerSurface>& ManagerSurface::Get()
	{
		static std::shared_ptr<ManagerSurface> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<ManagerSurface>(new ManagerSurface());
		}
		return manager;
	}

	ManagerSurface::~ManagerSurface()
	{
		LOG(Loglvl::debug, "[ManagerSurface::~ManagerSurface]");
	}

	std::shared_ptr<DataSurface> ManagerSurface::GetSurface(const SurfaceId& surfaceId)
	{
		return surface_[surfaceId];
	}
}
