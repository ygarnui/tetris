#pragma once

#include <interface_manager_asset_render.h>

#include <string>

namespace tetris
{
	/*!
	\brief Tells the renderer where shader sources and the shader cache live.

	Both paths are baked in by CMake and point into the source tree, so a shader can be
	edited and picked up by restarting the application instead of rebuilding it.
	*/
	class AssetManager : public general::InterfaceManagerAssetRender
	{
	public:
		const std::string& GetAssetPath() const override
		{
			return asset_path_;
		}

		const std::string& GetShaderCachePath() const override
		{
			return shader_cache_path_;
		}

	private:
		std::string asset_path_ = TETRIS_SHADER_PATH;
		std::string shader_cache_path_ = TETRIS_SHADER_CACHE_PATH;
	};
}
