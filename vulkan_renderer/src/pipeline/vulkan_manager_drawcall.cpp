#include "vulkan_manager_drawcall.h"

#include <logger_instance.h>
#include <generator_id.h>
#include <guard_next_id.h>
#include <collection_utils.h>

namespace render
{
	VulkanManagerDrawcall::VulkanManagerDrawcall() : next_id_(0)
	{

	}

	VulkanManagerDrawcall::~VulkanManagerDrawcall()
	{
		LOG(Loglvl::debug, "[VulkanManagerDrawcall::~VulkanManagerDrawcall]");
	}

	std::shared_ptr<VulkanManagerDrawcall> VulkanManagerDrawcall::Get()
	{
		static std::shared_ptr<VulkanManagerDrawcall> manager;
		if (!manager || manager->NeedReinit())
		{
			manager = std::shared_ptr<VulkanManagerDrawcall>(new VulkanManagerDrawcall());
		}
		return manager;
	}

	DrawcallId VulkanManagerDrawcall::CreateDrawCall()
	{
		auto nextId = GeneratorId::GenerateUniqueId<DrawcallId>(next_id_);
		auto guard = utils::GuardResize::MayBeResize(
			next_id_,
			[](const std::shared_ptr<Drawcall>& val) { return bool(!val); },
			drawcalls_);

		drawcalls_[nextId] = std::make_shared<VulkanDrawcall>();

		next_id_++;

		return nextId;
	}

	[[nodiscard]] std::shared_ptr<Drawcall> VulkanManagerDrawcall::GetDrawcall(const DrawcallId& drawcallId)
	{
		return drawcalls_[drawcallId];
	}

	void VulkanManagerDrawcall::DeleteDrawcall(const DrawcallId& drawcallId)
	{
		drawcalls_[drawcallId] = nullptr;
	}

	void VulkanManagerDrawcall::AddCommandInDrawCall(std::shared_ptr<CmdBaseAbstact> command, const DrawcallId& drawcallId)
	{
		drawcalls_[drawcallId]->AddCommand(command);
	}
}
