#pragma once

#include "cmd_calls.h"
#include "graphics_id.h"
#include "drawcall.h"

#include <memory>

namespace render
{
	class ManagerDrawcall
	{
	public:
		[[nodiscard]] virtual DrawcallId CreateDrawCall() = 0;
		[[nodiscard]] virtual std::shared_ptr<Drawcall> GetDrawcall(const DrawcallId& idDrawcall) = 0;

		virtual void DeleteDrawcall(const DrawcallId& idDrawcall) = 0;
		virtual void AddCommandInDrawCall(std::shared_ptr<CmdBaseAbstact> cmd, const DrawcallId& idDrawcall) = 0;
	};
}
