#pragma once

#include <interface_manager_asset_render.h>

#include <string>

namespace tetris
{
	/*!
	\brief Tells the renderer where shader sources, textures and the shader cache live.

	A debug build points into the source tree, so a shader or texture can be edited and
	picked up by restarting the application instead of rebuilding it. A release build has no
	source tree on the machine it runs on, so it points at an "assets" folder shipped next to
	the executable instead (see the post-build copy step in app/CMakeLists.txt). See
	asset_manager.cpp for exactly how each is resolved.
	*/
	class AssetManager : public general::InterfaceManagerAssetRender
	{
	public:
		AssetManager();

		const std::string& GetAssetPath() const override
		{
			return asset_path_;
		}

		const std::string& GetShaderCachePath() const override
		{
			return shader_cache_path_;
		}

		/*!
		\brief Where the button/screen textures live.

		Not part of InterfaceManagerAssetRender - texture loading is driven from app code
		(main.cpp), not from the engine, so this has no need to be virtual/overridable.
		*/
		const std::string& GetTexturePath() const
		{
			return texture_path_;
		}

	private:
		std::string asset_path_;
		std::string texture_path_;
		std::string shader_cache_path_;
	};
}
