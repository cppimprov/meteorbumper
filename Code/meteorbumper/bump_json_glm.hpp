#pragma once

#include <glm/glm.hpp>
#include <json.hpp>

namespace nlohmann
{

	template<glm::length_t S, class T, glm::qualifier Q>
	struct adl_serializer<glm::vec<S, T, Q>>
	{
		static glm::vec<S, T, Q> from_json(const json& j)
		{
			if (!j.is_array())
				throw json::type_error::create(302, std::string("parsing glm::vec: type must be array, found ") + j.type_name());
			
			if (j.size() != S)
				throw json::out_of_range::create(401, std::string("parsing glm::vec: array has invalid size"));

			auto v = glm::vec<S, T, Q>();

			for (auto i = glm::length_t{ 0 }; i != S; ++i)
				v[i] = j[i];
			
			return v;
		}

		static void to_json(json& , glm::vec<S, T, Q> )
		{
			throw std::runtime_error("implement me!");
		}
	};

	template<glm::length_t C, glm::length_t R, class T, glm::qualifier Q>
	struct adl_serializer<glm::mat<C, R, T, Q>>
	{
		static glm::mat<C, R, T, Q> from_json(const json& j)
		{
			if (!j.is_array())
				throw json::type_error::create(302, std::string("parsing glm::mat: type must be array, found ") + j.type_name());
			
			if (j.size() != C * R)
				throw json::out_of_range::create(401, std::string("parsing glm::mat: array has invalid size"));

			auto m = glm::mat<C, R, T, Q>();

			for (auto c = glm::length_t{ 0 }; c != C; ++c)
				for (auto r = glm::length_t{ 0 }; r != R; ++r)
					m[c][r] = j[c * R + r];
			
			return m;
		}

		static void to_json(json& , glm::mat<C, R, T, Q> )
		{
			throw std::runtime_error("implement me!");
		}
	};

} // nlohmann
