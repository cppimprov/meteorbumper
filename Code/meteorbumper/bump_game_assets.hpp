#pragma once

#include "bump_font_asset.hpp"
#include "bump_sdl_mixer_chunk.hpp"
#include "bump_gl_shader.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace bump
{
	
	namespace game
	{

		class app;

		class font_metadata
		{
		public:

			std::string m_name;
			std::string m_filename;
			std::uint32_t m_size_pixels_per_em;
		};

		class sound_metadata
		{
		public:

			std::string m_name;
			std::string m_filename;
		};

		class shader_metadata
		{
		public:

			std::string m_name;
			std::vector<std::string> m_filenames;
		};

		class assets
		{
		public:

			std::unordered_map<std::string, font::font_asset> m_fonts;
			std::unordered_map<std::string, sdl::mixer_chunk> m_sounds;
			std::unordered_map<std::string, gl::shader_program> m_shaders;
		};

		assets load_assets(app& app, 
			std::vector<font_metadata> const& fonts,
			std::vector<sound_metadata> const& sounds,
			std::vector<shader_metadata> const& shaders
			// ...
		);
		
	} // game
	
} // bump