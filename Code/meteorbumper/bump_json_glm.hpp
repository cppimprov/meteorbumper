#pragma once

#include <glm/glm.hpp>
#include <json.hpp>

namespace nlohmann
{

	template<std::size_t S, class T, glm::qualifier Q>
	struct adl_serializer<glm::vec<S, T, Q>>
	{
		static glm::vec<S, T, Q> from_json(const json& j)
		{
			if (!j.is_array())
				throw json::type_error::create(302, std::string("parsing glm::vector<>: type must be array, found ") + j.type_name());
			
			if (j.size() != S)
				throw json::out_of_range::create(401, std::string("parsing glm::vector<>: array has invalid size"));

			auto v = glm::vec<S, T, Q>();

			for (auto i = std::size_t{ 0 }; i != S; ++i)
				v[i] = j[i];
			
			return v;
		}

		static void to_json(json& j, glm::vec<S, T, Q> v)
		{
			for (auto e : v)
				j.push_back(e);
		}
	};

	// todo: matrices

} // nlohmann
