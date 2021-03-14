#pragma once

#include "bump_gl_texture.hpp"

#include <array>
#include <string>

namespace bump
{
	
	gl::texture_cubemap load_gl_cubemap_texture_from_files(std::array<std::string, 6> const& files);
	
} // bump
