#pragma once
#include <cfloat>

namespace utils
{
	class CollectionUtils
	{
	public:
		template<typename T, typename F>
		[[nodiscard]] static size_t FindNextId(const F& f, T* data, const size_t size, const size_t statr)
		{
			for (size_t i = statr; i < size; i++)
			{
				if (f(data[i]))
				{
					return i;
				}
			}

			return size;
		}
	};
}
