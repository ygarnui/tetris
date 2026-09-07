#pragma once

#include "render_id.h"

#include <atomic>

namespace render
{
	class GeneratorId
	{
	public:
		template<typename U>
		[[nodiscard]] static U GenerateUniqueId()
		{
			static std::atomic<uint64_t> newId = 0;
			return U(static_cast<uint64_t>(newId++));
		}

		template<typename T>
		[[nodiscard]] static RenderId GenerateId(T newId)
		{
			return RenderId(static_cast<uint64_t>(newId));
		}

		template<typename U, typename T>
		[[nodiscard]] static U GenerateUniqueId(T newId)
		{
			return U(static_cast<uint64_t>(newId));
		}

		template<typename U>
		[[nodiscard]] static U GenerateInvalidId()
		{
			return U(static_cast<uint64_t>(-1));
		}

		template<typename T>
		[[nodiscard]] static RenderId GenerateCollectionId(T newId)
		{
			return RenderId(static_cast<uint64_t>(newId.size()));
		}

		[[nodiscard]] static RenderId GenerateInvalidId()
		{
			return RenderId(-1);
		}
	};
}
