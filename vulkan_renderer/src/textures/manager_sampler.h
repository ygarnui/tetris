#pragma once

#include "../manager_base.h"
#include "../manager_device.h"

#include <vector.h>
#include <interface_image.h>

namespace render
{
	class ManagerSampler : public ManagerBase
	{
	public:
		ManagerSampler();

		~ManagerSampler();

		static std::shared_ptr<ManagerSampler>& Get();

		ManagerSampler(const ManagerSampler&) = delete;
		ManagerSampler(ManagerSampler&&) = delete;

		ManagerSampler& operator= (const ManagerSampler&) = delete;
		ManagerSampler& operator= (ManagerSampler&&) = delete;

		[[nodiscard]] std::shared_ptr<DataSampler> GetSamplerData(
			const LogicalDeviceId& logicalDeviceId,
			const SamplerId& samplerId) const;

		[[nodiscard]] SamplerId CreateSampler(
			const LogicalDeviceId& logicalDeviceId,
			const image::SamplerCreateInfo& createInfo);

		[[nodiscard]] SamplerId FindSamplerId(
			const LogicalDeviceId& logicalDeviceId,
			const image::SamplerCreateInfo& createInfo) const;

		void DeleteSampler(
			const LogicalDeviceId& logicalDeviceId,
			SamplerId& samplerId);

	private:

		size_t next_sampler_id_ = 0;

		struct SamplerInfo
		{
			image::SamplerCreateInfo create_info;
			std::shared_ptr<DataSampler> sampler;
		};

		std::map<LogicalDeviceId, Vector<SamplerInfo, SamplerId>> sampler_infos_;
		std::map<SamplerId, uint32_t> sampler_usage_;
	};
}
