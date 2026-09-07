#include "reader_shader.h"

#include "../vulkan_manager_assets.h"
#include <logger_instance.h>

#include <fstream>

namespace render
{
    std::string ReaderShader::ReadFile(const std::string& filepath)
    {
        std::string pathToAssets;
        auto pos = filepath.find(".h");
        if (pos < filepath.size())
        {
            pathToAssets = ManagerAssetsVulkan::GetInterface()->GetAssetPath();
        }
        std::ifstream file(pathToAssets + filepath, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            LOGEXC(std::runtime_error, "[ReaderShader::ReadFile] failed to open file: " + filepath);
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
