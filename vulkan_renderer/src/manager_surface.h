#pragma once

#include "struct_data.h"
#include "device_property.h"
#include "manager_device.h"
#include "manager_base.h"

#include <vector.h>
#include <map>
#include <functional>
#include <memory>

namespace render
{
	class ManagerSurface : public ManagerBase
	{
	public:
		static std::shared_ptr<ManagerSurface>& Get();

		ManagerSurface(const ManagerSurface&) = delete;
		ManagerSurface(ManagerSurface&&) = delete;

		ManagerSurface& operator= (const ManagerSurface&) = delete;
		ManagerSurface& operator= (ManagerSurface&&) = delete;

		~ManagerSurface();

		[[nodiscard]] SurfaceId AddSurface(
			std::shared_ptr<DataInstance> instance,
			const std::function<DataResult(std::shared_ptr<DataInstance>, std::shared_ptr<DataSurface>)>& createWindowSurface,
			bool externalDeleter);

		void DeleteSurface(SurfaceId& surfaceId);

		[[nodiscard]] std::shared_ptr<DataSurface> GetSurface(const SurfaceId& surfaceId);

	private:
		ManagerSurface();

		Vector<std::shared_ptr<DataSurface>, SurfaceId> surface_;

		std::vector<std::function<DataResult(std::shared_ptr<DataInstance>, std::shared_ptr<DataSurface>)>> create_window_surface_;
	};
}
