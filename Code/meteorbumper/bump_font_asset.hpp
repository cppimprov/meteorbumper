#pragma once

#include "bump_font_ft_font.hpp"
#include "bump_font_hb_font.hpp"
#include "bump_hash.hpp"

#include <cstdint>
#include <string>

namespace bump
{
	
	namespace font
	{
		
		class font_asset_key
		{
		public:

			std::string m_name;
			std::uint32_t m_size;
		};

		inline bool operator==(font_asset_key const& a, font_asset_key const& b)
		{
			return std::tie(a.m_name, a.m_size) == std::tie(b.m_name, b.m_size);
		}
		
		class font_asset_key_hash
		{
		public:

			std::size_t operator()(font_asset_key const& key) const
			{
				return hash_value(std::tie(key.m_name, key.m_size));
			}
		};

		class font_asset
		{
		public:

			font::ft_font m_ft_font;
			font::hb_font m_hb_font;
		};
		
	} // font
	
} // bump