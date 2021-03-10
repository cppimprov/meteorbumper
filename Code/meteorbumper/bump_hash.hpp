#pragma once

#include <cstdlib>
#include <tuple>

namespace bump
{
	
	inline std::size_t combine_hashes(std::size_t a, std::size_t b)
	{
		return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
	}

	template<class... Hashes>
	inline std::size_t combine_hashes(std::size_t a, std::size_t b, Hashes... others)
	{
		return combine_hashes(combine_hashes(a, b), others...);
	}

	template<class T>
	std::size_t hash_value(T const& t)
	{
		return std::hash<T>()(t);
	}

	template<class T> class tuple_hash;
	
	template<class... Ts>
	class tuple_hash<std::tuple<Ts...>>
	{
	public:

		size_t operator()(std::tuple<Ts...> const& t) const
		{
			return std::apply([] (auto const&... v) { return combine_hashes(hash_value(v)...); }, t);
		}
	};

	template<class... Args>
	std::size_t hash_value(std::tuple<Args...> const& t)
	{
		return tuple_hash<std::tuple<Args...>>()(t);
	}
	
} // bump