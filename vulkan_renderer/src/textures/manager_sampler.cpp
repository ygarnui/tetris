#include "manager_sampler.h"

#include "creator_texture_sampler.h"

#include <guard_next_id.h>
#include <generator_id.h>

namespace render
{
	ManagerSampler::ManagerSampler()
	{

	}

	ManagerSampler::~ManagerSampler()
	{
		LOG(Loglvl::debug, "[ManagerSampler::~ManagerSampler]");
	}

	std::shared_ptr<ManagerSampler>& ManagerSampler::Get()
	{
		static std::shared_ptr<ManagerSampler> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<ManagerSampler>(new ManagerSampler);
		}
		return manager;
	}

	std::shared_ptr<DataSampler> ManagerSampler::GetSamplerData(const LogicalDeviceId& logicalDeviceId, const SamplerId& samplerId) const
	{
		auto foundSamplerInfos = sampler_infos_.find(logicalDeviceId);
		if (foundSamplerInfos == sampler_infos_.end())
		{
			return nullptr;
		}

		return foundSamplerInfos->second[samplerId].sampler;
	}

	SamplerId ManagerSampler::CreateSampler(const LogicalDeviceId& logicalDeviceId, const image::SamplerCreateInfo& createInfo)
	{
		const SamplerId foundSampler = FindSamplerId(logicalDeviceId, createInfo);
		if (foundSampler.IsValid())
		{
			sampler_usage_[foundSampler]++;
			return foundSampler;
		}

		Vector<SamplerInfo, SamplerId>& samplerInfos = sampler_infos_[logicalDeviceId];

		const SamplerId samplerId = GeneratorId::GenerateUniqueId<SamplerId>(next_sampler_id_);

		auto guardResize = utils::GuardResize::MayBeResize(
			next_sampler_id_,
			[](const SamplerInfo& samplerInfo) { return !samplerInfo.sampler; },
			samplerInfos);

		SamplerInfo samplerInfo{};
		samplerInfo.create_info = createInfo;

		samplerInfo.sampler = CreatorTextureSampler::CreateTextureSampler(
			ManagerDevice::Get()->GetLogicalDevice(logicalDeviceId),
			createInfo);

		samplerInfos[samplerId] = samplerInfo;
		
		sampler_usage_[samplerId]++;

		return samplerId;
	}

	SamplerId ManagerSampler::FindSamplerId(const LogicalDeviceId& logicalDeviceId, const image::SamplerCreateInfo& createInfo) const
	{
		auto foundSamplerInfos = sampler_infos_.find(logicalDeviceId);
		if (foundSamplerInfos == sampler_infos_.end())
		{
			return GeneratorId::GenerateInvalidId<SamplerId>();
		}

		const Vector<SamplerInfo, SamplerId>& samplerInfos = foundSamplerInfos->second;
		for (size_t samplerIndex = 0; samplerIndex < samplerInfos.size(); samplerIndex++)
		{
			const SamplerId samplerId = GeneratorId::GenerateUniqueId<SamplerId>(samplerIndex);

			if (samplerInfos[samplerId].sampler 
				&& samplerInfos[samplerId].create_info.addressMode == createInfo.addressMode
				&& samplerInfos[samplerId].create_info.borderColor == createInfo.borderColor
				&& samplerInfos[samplerId].create_info.filter == createInfo.filter
				&& samplerInfos[samplerId].create_info.maxAnisotropy == createInfo.maxAnisotropy
				&& samplerInfos[samplerId].create_info.mipmapMode == createInfo.mipmapMode)
			{
				return samplerId;
			}
		}

		return GeneratorId::GenerateInvalidId<SamplerId>();
	}

	void ManagerSampler::DeleteSampler(const LogicalDeviceId& logicalDeviceId, SamplerId& samplerId)
	{
		sampler_usage_[samplerId]--;

		if (sampler_usage_[samplerId] > 0)
		{
			return;
		}

		auto foundSamplerInfos = sampler_infos_.find(logicalDeviceId);
		if (foundSamplerInfos == sampler_infos_.end())
		{
			return;
		}

		next_sampler_id_ = std::min(next_sampler_id_, samplerId.GetId());

		Vector<SamplerInfo, SamplerId>& samplerInfos = foundSamplerInfos->second;
		samplerInfos[samplerId] = {};
	}
}
