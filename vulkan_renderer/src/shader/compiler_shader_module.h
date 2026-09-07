#pragma once

#include <render_id.h>

#include "../converter_description.h"

#include <data_shader_module_reflection.h>
#include <spirv_cross/spirv_reflect.hpp>

#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>

namespace render
{
	class CompilerShaderModule
	{
	public:
		[[nodiscard]] static std::string Compile(
			const std::string& debugShaderName,
			const std::string& code,
			description::ShaderType type);

		[[nodiscard]] static std::string ReadCacheFile(
			const std::filesystem::path& shaderFilepath,
			const std::filesystem::path& cacheDirectory);

		static void WriteCacheFile(
			const std::filesystem::path& shaderFilepath,
			const std::filesystem::path& cacheDirectory,
			const std::string& spv);

		static DataShaderModuleReflection Reflect(const std::string& spv, const description::ShaderType type);

	private:
		[[nodiscard]] static description::Format convertSpirTypeToCoordsType(const spirv_cross::SPIRType type);

		[[nodiscard]] static description::Format convertSpirTypeToCoordsType8Bits(const spirv_cross::SPIRType type);
		[[nodiscard]] static description::Format convertSpirTypeToCoordsType16Bits(const spirv_cross::SPIRType type);
		[[nodiscard]] static description::Format convertSpirTypeToCoordsType32Bits(const spirv_cross::SPIRType type);
	};
}
