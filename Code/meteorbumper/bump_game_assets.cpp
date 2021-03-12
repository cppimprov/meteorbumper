#include "bump_game_assets.hpp"

#include "bump_die.hpp"
#include "bump_ends_with.hpp"
#include "bump_game_app.hpp"
#include "bump_log.hpp"

#include <fstream>

namespace bump
{
	
	namespace game
	{

		namespace
		{

			std::string read_file_to_string(std::string const& filename)
			{
				auto file = std::ifstream(filename, std::ios::binary); // todo: widen filename for windows!
				return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
			}

			GLenum get_shader_type(std::string const& filename)
			{
				if (ends_with(filename, ".vert"))
					return GL_VERTEX_SHADER;

				if (ends_with(filename, ".tesc"))
					return GL_TESS_CONTROL_SHADER;

				if (ends_with(filename, ".tese"))
					return GL_TESS_EVALUATION_SHADER;

				if (ends_with(filename, ".geom"))
					return GL_GEOMETRY_SHADER;

				if (ends_with(filename, ".frag"))
					return GL_FRAGMENT_SHADER;

				log_error("Unrecognised shader file extension: " + filename);
				die();

				return GL_VERTEX_SHADER;
			}

		} // unnamed
		
		assets load_assets(app& app, 
			std::vector<font_metadata> const& fonts,
			std::vector<sound_metadata> const& sounds,
			std::vector<shader_metadata> const& shaders,
			std::vector<model_metadata> const& models)
		{
			auto out = assets();

			// load fonts:
			{
				for (auto const& metadata : fonts)
				{
					auto ft_font = font::ft_font(app.m_ft_context.get_handle(), "data/fonts/" + metadata.m_filename);
					ft_font.set_pixel_size(metadata.m_size_pixels_per_em);

					auto hb_font = font::hb_font(ft_font.get_handle());

					if (!out.m_fonts.insert({ metadata.m_name, font::font_asset{ std::move(ft_font), std::move(hb_font) } }).second)
					{
						log_error("load_assets(): duplicate font id: " + metadata.m_name);
						die();
					}
				}
			}

			// load sounds:
			{
				for (auto const& metadata : sounds)
				{
					auto chunk = sdl::mixer_chunk("data/sounds/" + metadata.m_filename);

					if (!out.m_sounds.insert({ metadata.m_name, std::move(chunk) }).second)
					{
						log_error("load_assets(): duplicate sound id: " + metadata.m_name);
						die(); 
					}
				}
			}

			// load shaders:
			{
				for (auto const& metadata : shaders)
				{
					auto objects = std::vector<gl::shader_object>();

					for (auto const& file : metadata.m_filenames)
					{
						auto source = read_file_to_string("data/shaders/" + file);
						auto type = get_shader_type(file);

						auto object = gl::shader_object(type);
						object.set_source(source);

						if (!object.compile())
						{
							log_error("Failed to compile shader object: " + file + " for shader asset: " + metadata.m_name);
							log_error(object.get_log());
						}

						objects.push_back(std::move(object));
					}

					die_if(!std::all_of(objects.begin(), objects.end(), [] (gl::shader_object const& o) { return o.is_compiled(); }));

					auto shader = gl::shader_program();

					for (auto const& object : objects)
						shader.attach(object);
					
					if (!shader.link())
					{
						log_error("Failed to link shader program: " + metadata.m_name);
						log_error(shader.get_log());
						die();
					}

					for (auto const& object : objects)
						shader.detach(object);
					
					if (!out.m_shaders.insert({ metadata.m_name, std::move(shader) }).second)
					{
						log_error("load_assets(): duplicate shader id: " + metadata.m_name);
						die();
					}
				}
			}

			// load models:
			{
				for (auto const& metadata : models)
				{
					auto model = load_mbp_model_json("data/models/" + metadata.m_filename);

					if (!out.m_models.insert({ metadata.m_name, std::move(model) }).second)
					{
						log_error("load_assets(): duplicate model id: " + metadata.m_name);
						die();
					}
				}
			}

			return out;
		}
		
	} // game
	
} // bump