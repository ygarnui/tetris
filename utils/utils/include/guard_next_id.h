#pragma once

#include "collection_utils.h"

#include <functional>
#include <memory>

namespace utils
{

	class GuardResize
	{
	public:

		GuardResize(size_t& val, std::function<size_t()> func) :
			func_(func),
			next_id_(val)
		{

		}

		~GuardResize()
		{
			if (func_)
			{
				next_id_ = func_();
			}
			else
			{
				next_id_++;
			}
		}

		template<typename T, typename F, typename... Args>
		static GuardResize MayBeResize(size_t& num, const F& f, T& vec, Args&... args)
		{
			if (vec.size() <= num)
			{
				Resize(num + 1, vec, args...);

				return GuardResize(num, std::function<size_t()>());
			}
			return GuardResize(num, [&]() {return CollectionUtils::FindNextId(f, vec.data(), vec.size(), num); });
		}

		template<typename T, typename... Args>
		static void Resize(const size_t& num, T& t, Args&... args)
		{
			t.resize(num);
			Resize(num, args...);
		}

		template<typename T>
		static void Resize(const size_t& num, T& t)
		{
			t.resize(num);
		}

	private:

		std::function<size_t()> func_;
		size_t& next_id_;
	};
}
