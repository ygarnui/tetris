#pragma once

#include <cmd_calls.h>

#include <memory>
#include <vector>

namespace render
{
	class Drawcall
	{
	public:
		virtual void AddCommand(std::shared_ptr<CmdBaseAbstact> command) = 0;
		[[nodiscard]] virtual const std::vector<std::shared_ptr<CmdBaseAbstact>>& GetCommands() const = 0;
	};
}