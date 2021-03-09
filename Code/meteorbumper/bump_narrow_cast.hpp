#pragma once

#include "bump_die.hpp"

#include <type_traits>

namespace bump
{
	
	template<class T, class U,
		class = std::enable_if_t<std::is_integral<T>::value && std::is_integral<U>::value>>
	T narrow_cast(U u)
	{
		auto t = static_cast<T>(u);
		die_if(static_cast<U>(t) != u);
		die_if(std::is_signed_v<T> != std::is_signed_v<U> && ((t < T{ 0 }) != (u < U{ 0 })));
		return t;
	}
	
} // bump