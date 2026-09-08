#include "asset_manager.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <filesystem>
#include <stdexcept>

namespace tetris
{

namespace
{
	/*!
	\brief Directory the running executable's own .exe file lives in.
	*/
	std::filesystem::path GetExecutableDirectory()
	{
		wchar_t buffer[MAX_PATH];
		const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
		if (length == 0 || length == MAX_PATH)
		{
			throw std::runtime_error("failed to determine the executable's own path");
		}

		return std::filesystem::path(buffer).parent_path();
	}
}

AssetManager::AssetManager()
{
#ifdef _DEBUG
	asset_path_ = TETRIS_SHADER_PATH;
	texture_path_ = TETRIS_TEXTURE_PATH;
	shader_cache_path_ = TETRIS_SHADER_CACHE_PATH;
#else
	const std::filesystem::path exeDirectory = GetExecutableDirectory();
	asset_path_ = (exeDirectory / "assets" / "shaders").string();
	texture_path_ = (exeDirectory / "assets" / "textures").string();
	shader_cache_path_ = (exeDirectory / "shader_cache").string();
#endif

	std::filesystem::create_directories(shader_cache_path_);
}

}
