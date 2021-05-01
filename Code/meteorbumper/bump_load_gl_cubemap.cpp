#include "bump_load_gl_cubemap.hpp"

#include "bump_log.hpp"
#include "bump_die.hpp"

#include <stb_image.h>
#include <stb_image_write.h>

#include <algorithm>

namespace bump
{

	namespace
	{

		void vflip(stbi_uc* pixels, int width, int height, int channels)
		{
			for (auto y = 0; y != height / 2; ++y)
			{
				auto src = pixels + y * width * channels;
				auto dst = pixels + ((height - 1) - y) * width * channels;
				std::swap_ranges(src, src + width * channels, dst);
			}
		}

		void hflip(stbi_uc* pixels, int width, int height, int channels)
		{
			for (auto y = 0; y != height; ++y)
			{
				auto row = pixels + y * width * channels;

				for (auto x = 0; x != width / 2; ++x)
				{
					auto src = row + x * channels;
					auto dst = row + ((width - 1) - x) * channels;
					std::swap_ranges(src, src + channels, dst);
				}
			}
		}

	} // unnamed

	gl::texture_cubemap load_gl_cubemap_texture_from_files(std::array<std::string, 6> const& files)
	{
		auto out = gl::texture_cubemap();

		// OpenGL uses "Renderman" style coordinates for cubemaps. :(
		// So we have to flip some textures horizontally and vertically to compensate.

		// pos_x, neg_x, pos_y, neg_y, pos_z, neg_z
		auto flip = std::array<bool, 6>{  true, true, false, false, true, true };

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

			if (flip[i])
			{
				vflip(pixels, width, height, channels);
				hflip(pixels, width, height, channels);
			}

			out.set_data(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int)i, { width, height }, GL_SRGB8, 
				gl::make_texture_data_source(GL_RGB, pixels));

			stbi_image_free(pixels);
		}

		out.set_min_filter(GL_LINEAR_MIPMAP_LINEAR);
		out.set_mag_filter(GL_LINEAR);
		out.generate_mipmaps(); // todo: test mipmaps!

		return out;
	}
	
} // bump