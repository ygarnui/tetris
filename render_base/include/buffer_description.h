#pragma once

#include "format.h"
#include "sampler_create_info.h"

#include <unordered_map>
#include <filesystem>
#include <string>
#include <vector>

namespace description
{
	enum class IndexType
	{
		UINT16 = 0,
		UINT32,
		NONE
	};

	/*!
	\brief Shader stages.

	The ray tracing stages are ordered raygen, miss, closest hit on purpose: a shader program
	keeps its modules in a map keyed by this enum, and the ray tracing pipeline relies on that
	order to lay out its shader groups the same way the shader binding table addresses them.
	*/
	enum class ShaderType
	{
		VERTEX = 0,
		TESSELLATION_CONTROL,
		TESSELLATION_EVALUATION,
		GEOMETRY,
		COMPUTE,
		FRAGMENT,
		RAY_GENERATION,
		RAY_MISS,
		RAY_CLOSEST_HIT,
		RAY_ANY_HIT,
		RAY_INTERSECTION,
		NONE,
		COUNT = NONE
	};

	class ShaderTypeMap
	{
	public:
		static const std::unordered_map<std::string, ShaderType>& GetMap()
		{
			if (shaderTypes.size() != static_cast<size_t>(ShaderType::COUNT))
			{
				LOGEXC(std::runtime_error, "Size of ShaderTypeMap must be equal to ShaderType::COUNT");
			}

			return shaderTypes;
		}

	private:
		static inline const std::unordered_map<std::string, ShaderType> shaderTypes
		{
			{ "VERTEX", ShaderType::VERTEX },
			{ "TESSELLATION_CONTROL", ShaderType::TESSELLATION_CONTROL },
			{ "TESSELLATION_EVALUATION", ShaderType::TESSELLATION_EVALUATION },
			{ "GEOMETRY", ShaderType::GEOMETRY },
			{ "COMPUTE", ShaderType::COMPUTE },
			{ "FRAGMENT", ShaderType::FRAGMENT },
			{ "RAY_GENERATION", ShaderType::RAY_GENERATION },
			{ "RAY_MISS", ShaderType::RAY_MISS },
			{ "RAY_CLOSEST_HIT", ShaderType::RAY_CLOSEST_HIT },
			{ "RAY_ANY_HIT", ShaderType::RAY_ANY_HIT },
			{ "RAY_INTERSECTION", ShaderType::RAY_INTERSECTION },
		};
	};

	enum class TypeTopology
	{
		POINTS,
		LINE_LIST,
		LINE_STRIP,
		TRIANGLE_LIST,
		TRIANGLE_STRIP,
		PRIMITIVE_TOPOLOGY_PATCH_LIST,
		NONE,
		COUNT = NONE
	};

	class TypeTopologyMap
	{
	public:
		static const std::unordered_map<std::string, TypeTopology>& GetMap()
		{
			if (typesTopology.size() != static_cast<size_t>(TypeTopology::COUNT))
			{
				LOGEXC(std::runtime_error, "Size of TypeTopologyMap must be equal to TypeTopology::COUNT");
			}

			return typesTopology;
		}

	private:
		static inline const std::unordered_map<std::string, TypeTopology> typesTopology
		{
			{"POINTS", TypeTopology::POINTS},
			{"LINE_LIST", TypeTopology::LINE_LIST},
			{"LINE_STRIP", TypeTopology::LINE_STRIP},
			{"TRIANGLE_LIST", TypeTopology::TRIANGLE_LIST},
			{"TRIANGLE_STRIP", TypeTopology::TRIANGLE_STRIP},
			{"PRIMITIVE_TOPOLOGY_PATCH_LIST", TypeTopology::PRIMITIVE_TOPOLOGY_PATCH_LIST}
		};
	};

	enum class TypeLine : int
	{
		DASHED,
		SOLID,
		SIMPLE,
		DOT,
		COUNT,
		NONE = COUNT
	};

	enum class TypePoint
	{
		TRIANGLE,
		CIRCLE,
		COUNT,
		NONE = COUNT
	};

	enum class TypePolygonMode
	{
		POLYGON_MODE_FILL = 0,
		POLYGON_MODE_LINE,
		POLYGON_MODE_POINT,
		POLYGON_MODE_FILL_RECTANGLE_NV,
		NONE,
		COUNT = NONE
	};

	class TypePolygonModeMap
	{
	public:
		static const std::unordered_map<std::string, TypePolygonMode>& GetMap()
		{
			if (typesPolygonMode.size() != static_cast<size_t>(TypePolygonMode::COUNT))
			{
				LOGEXC(std::runtime_error, "Size of TypePolygonModeMap must be equal to TypePolygonMode::COUNT");
			}

			return typesPolygonMode;
		}

	private:
		static inline const std::unordered_map<std::string, TypePolygonMode> typesPolygonMode
		{
			{"POLYGON_MODE_FILL", TypePolygonMode::POLYGON_MODE_FILL},
			{"POLYGON_MODE_LINE", TypePolygonMode::POLYGON_MODE_LINE},
			{"POLYGON_MODE_POINT", TypePolygonMode::POLYGON_MODE_POINT},
			{"POLYGON_MODE_FILL_RECTANGLE_NV", TypePolygonMode::POLYGON_MODE_FILL_RECTANGLE_NV},
		};
	};

	enum class AttachmentLoadOp
	{
		ATTACHMENT_LOAD_OP_LOAD = 0,
		ATTACHMENT_LOAD_OP_CLEAR,
		ATTACHMENT_LOAD_OP_DONT_CARE,
		NONE
	};

	enum class UniformType
	{
		COMBINED_IMAGE_SAMPLER = 0,
		UNIFORM_BUFFER,
		STORAGE_BUFFER,
		STORAGE_IMAGE,
		ACCELERATION_STRUCTURE,
		COUNT,
		NONE = COUNT
	};

	enum class DescriptorSetIndexType
	{
		WINDOW = 0,
		RENDER_PASS,
		BASE_MATERIAL,
		DERIVED_MATERIAL,
		OBJECT,
		COMPUTE,
		COUNT,
		NONE = COUNT
	};

	class DescriptorSetIndexTypeMap
	{
	public:
		static const std::unordered_map<std::string, DescriptorSetIndexType>& GetMap()
		{
			if (descriptorSetIndexType.size() != static_cast<size_t>(DescriptorSetIndexType::COUNT))
			{
				LOGEXC(std::runtime_error, "Size of DescriptorSetIndexTypeMap must be equal to SDescriptorSetIndexType::COUNT");
			}

			return descriptorSetIndexType;
		}

	private:
		static inline const std::unordered_map<std::string, DescriptorSetIndexType> descriptorSetIndexType
		{
			{"WINDOW", DescriptorSetIndexType::WINDOW},
			{"RENDER_PASS", DescriptorSetIndexType::RENDER_PASS},
			{"BASE_MATERIAL", DescriptorSetIndexType::BASE_MATERIAL},
			{"DERIVED_MATERIAL", DescriptorSetIndexType::DERIVED_MATERIAL},
			{"OBJECT", DescriptorSetIndexType::OBJECT},
			{"COMPUTE", DescriptorSetIndexType::COMPUTE},
		};
	};

	enum class VertexInputRate
	{
		VERTEX = 0,
		INSTANCE,
		COUNT
	};

	class VertexInputRateMap
	{
	public:
		static const std::unordered_map<std::string, VertexInputRate>& GetMap()
		{
			if (vertexInputRate.size() != static_cast<size_t>(VertexInputRate::COUNT))
			{
				LOGEXC(std::runtime_error, "Size of VertexInputRateMap must be equal to VertexInputRate::COUNT");
			}

			return vertexInputRate;
		}

	private:
		static inline const std::unordered_map<std::string, VertexInputRate> vertexInputRate
		{
			{"VERTEX", VertexInputRate::VERTEX},
			{"INSTANCE", VertexInputRate::INSTANCE},
		};
	};

	enum class AttachmentStoreOp
	{
		ATTACHMENT_STORE_OP_STORE = 0,
		ATTACHMENT_STORE_OP_DONT_CARE = 1,
		ATTACHMENT_STORE_OP_NONE = 1000301000,
		ATTACHMENT_STORE_OP_NONE_KHR = ATTACHMENT_STORE_OP_NONE,
		ATTACHMENT_STORE_OP_NONE_QCOM = ATTACHMENT_STORE_OP_NONE,
		ATTACHMENT_STORE_OP_NONE_EXT = ATTACHMENT_STORE_OP_NONE,
		ATTACHMENT_STORE_OP_MAX_ENUM = 0x7FFFFFFF
	};

	typedef uint64_t VertexAttributeFlags;

	// the parameters start with the last bit, so that adding them would be as simple as possible
	enum class VertexAttributeFlagBits : VertexAttributeFlags
	{
		POSITION = 1uLL << 0,
		NORMAL = 1uLL << 1,
		COLOR = 1uLL << 2,
		UV = 1uLL << 3,
		INSTANCE = 1uLL << 4,
		PARAMS4 = 1uLL << 59,
		PARAMS3 = 1uLL << 60,
		PARAMS2 = 1uLL << 61,
		PARAMS1 = 1uLL << 62,
		PARAMS0 = 1uLL << 63,
	};

	class VertexAttributeFlagBitsMap
	{
	public:
		static const std::unordered_map<std::string, VertexAttributeFlagBits>& GetMap()
		{
			if (vertexAttributeFlagBits.size() != 10) // VALUE MUST BE UPDATED if VertexAttributeFlagBits is changed
			{
				LOGEXC(std::runtime_error, "Size of VertexAttributeFlagBitsMap must be equal to VertexAttributeFlagBits::COUNT");
			}

			return vertexAttributeFlagBits;
		}

	private:
		static inline const std::unordered_map<std::string, VertexAttributeFlagBits> vertexAttributeFlagBits
		{
			{"POSITION", VertexAttributeFlagBits::POSITION},
			{"NORMAL", VertexAttributeFlagBits::NORMAL},
			{"COLOR", VertexAttributeFlagBits::COLOR},
			{"UV", VertexAttributeFlagBits::UV},
			{"INSTANCE", VertexAttributeFlagBits::INSTANCE},
			{"PARAMS4", VertexAttributeFlagBits::PARAMS4},
			{"PARAMS3", VertexAttributeFlagBits::PARAMS3},
			{"PARAMS2", VertexAttributeFlagBits::PARAMS2},
			{"PARAMS1", VertexAttributeFlagBits::PARAMS1},
			{"PARAMS0", VertexAttributeFlagBits::PARAMS0},
		};
	};

	/*!
	\brief Used just for vertex buffer input attributes.
	*/
	struct InputBindingDescription
	{
		uint32_t binding;
		VertexInputRate input_rate;
		VertexAttributeFlagBits vertex_attribute_flag_bit;
		std::vector<std::string> names;
	};

	struct ShaderDescription
	{
		std::filesystem::path filepath;
		ShaderType type = ShaderType::NONE;
		std::vector<description::InputBindingDescription> binding_descriptions;
	};

	enum class MemoryAccess
	{
		CPU = 0,
		GPU
	};

	enum class RenderPassType
	{
		FIRST_RENDER_PASS_IN_QUEUE = 0,
		DEFAULT_RENDER_PASS,
		GIZMO,
		MASK_FOR_STROKE,
		COUNT,
		NONE = COUNT
	};

	class RenderPassTypeMap
	{
	public:
		static const std::unordered_map<std::string, RenderPassType>& GetMap()
		{
			if (renderPassTypes.size() != static_cast<size_t>(RenderPassType::COUNT))
			{
				LOGEXC(std::runtime_error, "Size of RenderPassTypeMap must be equal to RenderPassType::COUNT");
			}

			return renderPassTypes;
		}

	private:
		static inline const std::unordered_map<std::string, RenderPassType> renderPassTypes
		{
			{"FIRST_RENDER_PASS_IN_QUEUE", RenderPassType::FIRST_RENDER_PASS_IN_QUEUE},
			{"DEFAULT_RENDER_PASS", RenderPassType::DEFAULT_RENDER_PASS},
			{"GIZMO", RenderPassType::GIZMO},
			{"MASK_FOR_STROKE", RenderPassType::MASK_FOR_STROKE}
		};
	};

	enum class TextureUsage
	{
		TEXTURE,
		COLOR_ATTACHMENT,
		DEPTH_ATTACHMENT,
		STORAGE_TEXTURE
	};

	struct ClearDepth
	{
		float clear_depth;
		uint32_t clear_stencil;
	};

	enum class ImageLayout
	{
		UNDEFINED = 0,
		GENERAL = 1,
		COLOR_ATTACHMENT_OPTIMAL = 2,
		DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
		DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
		SHADER_READ_ONLY_OPTIMAL = 5,
		TRANSFER_SRC_OPTIMAL = 6,
		TRANSFER_DST_OPTIMAL = 7,
		PRESENT_SRC_KHR = 1000001002,
	};

	enum class SampleCountFlagBits : uint32_t
	{
		SAMPLE_COUNT_1_BIT = 0x00000001,
		SAMPLE_COUNT_2_BIT = 0x00000002,
		SAMPLE_COUNT_4_BIT = 0x00000004,
		SAMPLE_COUNT_8_BIT = 0x00000008,
		SAMPLE_COUNT_16_BIT = 0x00000010,
		SAMPLE_COUNT_32_BIT = 0x00000020,
		SAMPLE_COUNT_64_BIT = 0x00000040,
		SAMPLE_COUNT_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	};

	struct RenderPassAttachmentDescription
	{
		Format format = Format::R8G8B8A8_UNORM;
		AttachmentLoadOp load_op = AttachmentLoadOp::ATTACHMENT_LOAD_OP_DONT_CARE;
		AttachmentStoreOp store_op = AttachmentStoreOp::ATTACHMENT_STORE_OP_STORE;
		ImageLayout initial_layout = ImageLayout::UNDEFINED;
		ImageLayout final_layout = ImageLayout::UNDEFINED;
		SampleCountFlagBits samples = SampleCountFlagBits::SAMPLE_COUNT_1_BIT;
		image::SamplerCreateInfo::AddressMode address_mode = image::SamplerCreateInfo::AddressMode::REPEAT;

		/*!
		\brief If set to true, the attachment description will be taken from the swapchain,
		and format variable will not be used..
		*/
		bool present_to_swapchain = false;
	};

	enum class UniformBufferType
	{
		UNIFORM,
		STORAGE
	};

	enum class FrameBufferType
	{
		SCREEN,
		MASK_FOR_STROKE
	};

	struct ImportImageDescription
	{
		std::string filename;
		image::SamplerCreateInfo::AddressMode address_mode = image::SamplerCreateInfo::AddressMode::REPEAT;

		explicit ImportImageDescription(const std::string& filename_) : filename(filename_) {}
		explicit ImportImageDescription(const std::string& filename_, image::SamplerCreateInfo::AddressMode address_mode_)
			: filename(filename_), address_mode(address_mode_) {}
	};
}
