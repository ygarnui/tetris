#pragma once

#include <drawcall.h>

#include <memory>
#include <vector>

namespace render
{
	class VulkanDrawcall : public Drawcall
	{
	public:
		VulkanDrawcall() = default;
		virtual ~VulkanDrawcall();

		void AddCommand(std::shared_ptr<CmdBaseAbstact> command) override;
		[[nodiscard]] const std::vector<std::shared_ptr<CmdBaseAbstact>>& GetCommands() const override;

	private:
		std::vector<std::shared_ptr<CmdBaseAbstact>> commands_;
		DrawcallId id_;
	};
}