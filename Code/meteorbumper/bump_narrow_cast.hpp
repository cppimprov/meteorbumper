#pragma once

#include "bump_die.hpp"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

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

	template<class RT, std::size_t S, class T = typename RT::value_type, class U, glm::qualifier Q,
		class = std::enable_if_t<std::is_integral_v<T> && std::is_integral_v<U>>>
	glm::vec<S, T, Q> narrow_cast(glm::vec<S, U, Q> u)
	{
		auto t = glm::vec<S, T, Q>(u);
		die_if(glm::vec<S, U, Q>(t) != u);
		die_if(std::is_signed_v<T> != std::is_signed_v<U> && glm::lessThan(t, glm::vec<S, T, Q>(0)) != glm::lessThan(u, glm::vec<S, U, Q>()));
		return t;
	}

} // bump