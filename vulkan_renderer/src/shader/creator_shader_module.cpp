#include "creator_shader_module.h"

#include <logger_instance.h>

namespace render
{
	std::shared_ptr<DataShaderModule> CreatorShaderModule::CreateShaderModule(
		std::shared_ptr<DataDevice> device,
		const std::string& code)
	{
		std::shared_ptr<DataShaderModule> shader(
			new DataShaderModule{ {}, device },
			[](DataShaderModule* p) {
				vkDestroyShaderModule(p->device->device, p->shader, nullptr);
				delete p;
			}
		);

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		auto res = vkCreateShaderModule(device->device, &createInfo, nullptr, &shader->shader);

		if (res != VK_SUCCESS)
		{
            LOGEXC(std::runtime_error, "failed to create shader module");
		}

		return shader;
	}
}
