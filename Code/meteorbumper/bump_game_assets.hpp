#pragma once

#include "bump_font_asset.hpp"
#include "bump_sdl_mixer_chunk.hpp"
#include "bump_gl_shader.hpp"
#include "bump_mbp_model.hpp"
#include "bump_gl_texture.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace bump
{
	
	namespace game
	{

		class app;

		struct font_metadata
		{
			std::string m_name;
			std::string m_filename;
			std::uint32_t m_size_pixels_per_em;
		};

		struct sound_metadata
		{
			std::string m_name;
			std::string m_filename;
		};

		struct shader_metadata
		{
			std::string m_name;
			std::vector<std::string> m_filenames;
		};

		struct model_metadata
		{
			std::string m_name;
			std::string m_filename;
		};

		struct cubemap_metadata
		{
			std::string m_name;
			std::array<std::string, 6u> m_filenames; // pos_x, neg_x, pos_y, neg_y, pos_z, neg_z
		};

		class assets
		{
		public:

			std::unordered_map<std::string, font::font_asset> m_fonts;
			std::unordered_map<std::string, sdl::mixer_chunk> m_sounds;
			std::unordered_map<std::string, gl::shader_program> m_shaders;
			std::unordered_map<std::string, mbp_model> m_models;
			std::unordered_map<std::string, gl::texture_cubemap> m_cubemaps;
		};

		assets load_assets(app& app, 
			std::vector<font_metadata> const& fonts,
			std::vector<sound_metadata> const& sounds,
			std::vector<shader_metadata> const& shaders,
			std::vector<model_metadata> const& models,
			std::vector<cubemap_metadata> const& cubemaps
			// ...
		);
		
	} // game
	
} // bump