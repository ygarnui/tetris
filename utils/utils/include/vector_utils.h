#pragma once

#include <stdexcept>

namespace utils
{
	template<typename T>
	static void FastErase(T& vec, size_t pos)
	{
		if (pos < vec.size() - 1)
		{
			std::swap(vec[pos], vec.back());
		}
		vec.erase(vec.end() - 1);
	}

	template<typename T>
	static void UpdateVector(std::vector<T>& data, size_t offset, const std::vector<T>& vec, size_t size)
	{
		data.resize(size);
		if (data.size() < offset + vec.size())
		{
			throw std::runtime_error("invalid length update vector");
		}
		for (size_t i = 0; i < vec.size(); i++)
		{
			data[offset + i] = vec[i];
		}
	}
}
