#pragma once

#include <functional>
#include <initializer_list>
#include <optional>
#include <unordered_map>

namespace bump
{
	
	template<class T1, class T2>
	class pair_map
	{
	public:

		using pair_type = std::pair<T1, T2>;

		explicit pair_map(std::initializer_list<pair_type> values)
		{
			for (auto const& p : values)
			{
				m_map_1.emplace(p.first, p.second);
				m_map_2.emplace(p.second, p.first);
			}
		}

		std::optional<std::reference_wrapper<T1 const>> find_first(T2 key) const
		{
			auto entry = m_map_2.find(key);
			return (entry != m_map_2.end() ? entry->second : std::optional<std::reference_wrapper<T1 const>>());
		}

		std::optional<std::reference_wrapper<T2 const>> find_second(T1 key) const
		{
			auto entry = m_map_1.find(key);
			return (entry != m_map_1.end() ? entry->second : std::optional<std::reference_wrapper<T2 const>>());
		}

		T1 const& get_first(T2 const& key) const
		{
			return m_map_2.find(key)->second;
		}

		T2 const& get_second(T1 const& key) const
		{
			return m_map_1.find(key)->second;
		}

		std::unordered_map<T1, T2> m_map_1;
		std::unordered_map<T2, T1> m_map_2;
	};
	
} // bump