#pragma once

#include "buffer_description.h"
#include <glm/glm.hpp>

#include <vector>

namespace description
{
	struct RenderPassCreateInfo
	{
		description::RenderPassType type = description::RenderPassType::NONE;
		std::vector<glm::vec4> clearColors;
		std::vector<glm::vec4> clearColorsResolve;
		std::vector<description::RenderPassAttachmentDescription> colorAttachmentDescriptions;
		std::vector<description::RenderPassAttachmentDescription> colorAttachmentDescriptionsResolve;
		std::optional<description::ClearDepth> clearDepth;
		std::optional<description::RenderPassAttachmentDescription> depthAttachmentDescription;
	};
}
