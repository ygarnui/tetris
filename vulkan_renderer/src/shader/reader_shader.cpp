#include "reader_shader.h"

#include "../vulkan_manager_assets.h"
#include <logger_instance.h>

#include <fstream>

namespace render
{
    std::string ReaderShader::ReadFile(const std::string& filepath)
    {
        // A #include target (globals/raytracing.h and friends) is relative to the asset root
        // and needs it joined on; the top level shader (already the full path built by
        // ManagerShaderModule::addShader) does not.
        std::filesystem::path fullPath = filepath;
        const auto pos = filepath.find(".h");
        if (pos < filepath.size())
        {
            fullPath = std::filesystem::path(ManagerAssetsVulkan::GetInterface()->GetAssetPath()) / filepath;
        }

        std::ifstream file(fullPath, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            LOGEXC(std::runtime_error, "[ReaderShader::ReadFile] failed to open file: " + fullPath.string());
        }

        const size_t fileSize = (size_t)file.tellg();
        std::string buffer;
        buffer.resize(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    std::string ReaderShader::ReadFile(const std::filesystem::path& filepath)
    {
        return ReadFile(filepath.string());
    }
}
