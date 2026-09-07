#pragma once

#include <vector>
#include <string>
#include <filesystem>

namespace render
{
	class ReaderShader
	{
	public:
		[[nodiscard]] static std::string ReadFile(const std::string& filepath);

		[[nodiscard]] static std::string ReadFile(const std::filesystem::path& filepath);
	};
}
