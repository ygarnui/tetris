#include "shader_includer.h"

#include "reader_shader.h"

#include <array>

namespace render
{

shaderc_include_result* render::ShaderIncluder::GetInclude(
	const char* requestedSource,
	shaderc_include_type type,
	const char* requestingSource,
	size_t includeDepth)
{
	std::string filepath = requestedSource;
	std::string content = ReaderShader::ReadFile(filepath);

	std::array<std::string, 2>* userData = new std::array<std::string, 2>();
	(*userData)[0] = filepath;
	(*userData)[1] = content;

	shaderc_include_result* data = new shaderc_include_result();

	data->user_data = userData;
	data->source_name = userData->at(0).c_str();
	data->source_name_length = userData->at(0).size();
	data->content = userData->at(1).c_str();
	data->content_length = userData->at(1).size();

	return data;
}

void ShaderIncluder::ReleaseInclude(shaderc_include_result* data)
{
	delete static_cast<std::array<std::string, 2>*>(data->user_data);
	delete data;
}

}
