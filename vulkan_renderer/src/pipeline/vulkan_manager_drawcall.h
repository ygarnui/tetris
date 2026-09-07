#pragma once

#include "vulkan_drawcall.h"
#include "../manager_base.h"

#include <manager_drawcall.h>
#include <cmd_calls.h>
#include <graphics_id.h>

#include <vector.h>

#include <memory>

namespace render
{
	class VulkanManagerDrawcall : public ManagerDrawcall, public ManagerBase
	{
	public:

		VulkanManagerDrawcall();
		~VulkanManagerDrawcall() override;
		
		static std::shared_ptr<VulkanManagerDrawcall> Get();

		VulkanManagerDrawcall(const VulkanManagerDrawcall&) = delete;
		VulkanManagerDrawcall(VulkanManagerDrawcall&&) = delete;

		VulkanManagerDrawcall& operator= (const VulkanManagerDrawcall&) = delete;
		VulkanManagerDrawcall& operator= (VulkanManagerDrawcall&&) = delete;

		[[nodiscard]] DrawcallId CreateDrawCall() override;
		[[nodiscard]] std::shared_ptr<Drawcall> GetDrawcall(const DrawcallId& drawcallId) override;

		void DeleteDrawcall(const DrawcallId& drawcallId) override;
		void AddCommandInDrawCall(std::shared_ptr<CmdBaseAbstact> cmd, const DrawcallId& drawcallId) override;

	private:
		Vector<std::shared_ptr<Drawcall>, DrawcallId> drawcalls_;
		size_t next_id_;
	};
}
