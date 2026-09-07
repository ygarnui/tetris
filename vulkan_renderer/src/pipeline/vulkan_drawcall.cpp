#include "vulkan_drawcall.h"

namespace render
{
	VulkanDrawcall::~VulkanDrawcall()
	{

	}

	void VulkanDrawcall::AddCommand(std::shared_ptr<CmdBaseAbstact> command)
	{
		commands_.push_back(command);
	}

	const std::vector<std::shared_ptr<CmdBaseAbstact>>& VulkanDrawcall::GetCommands() const
	{
		return commands_;
	}
}
