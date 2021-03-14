#include "bump_load_gl_cubemap.hpp"

#include "bump_log.hpp"
#include "bump_die.hpp"

#include <stb_image.h>

namespace bump
{

	gl::texture_cubemap load_gl_cubemap_texture_from_files(std::array<std::string, 6> const& files)
	{
		auto out = gl::texture_cubemap();

		for (auto i = std::size_t{ 0 }; i != files.size(); ++i)
		{
			auto const& file = files[i];

			stbi_set_flip_vertically_on_load(true);

			auto width = 0;
			auto height = 0;
			auto channels = 0;
			auto pixels = stbi_load(file.c_str(), &width, &height, &channels, 0);

			if (!pixels)
			{
				log_error("stbi_load() failed: " + std::string(stbi_failure_reason()));
				die();
			}

			if (channels != 3)
			{
				log_error("load_gl_cubemap_texture_from_files(): image does not have 3 channels: " + file);
				die();
			}

			out.set_data(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int)i, { width, height }, GL_RGB8, 
				gl::make_texture_data_source(GL_RGB, pixels));

			stbi_image_free(pixels);
		}

		out.set_min_filter(GL_LINEAR);
		out.set_mag_filter(GL_LINEAR);
		//out.generate_mipmaps(); // todo: test mipmaps!

		return out;
	}
	
} // bump