#include "bump_gl_texture.hpp"

#include "bump_die.hpp"
#include "bump_gl_error.hpp"

namespace bump
{
	
	namespace gl
	{

		texture_2d::texture_2d()
		{
			auto id = GLuint{ 0 };
			glGenTextures(1, &id);
			die_if(!id);

			reset(id, [] (GLuint id) { glDeleteTextures(1, &id); });

			set_min_filter(GL_NEAREST);
			set_mag_filter(GL_NEAREST);
			set_wrap_mode(GL_CLAMP_TO_EDGE);
			set_anisotropy(1.f);
		}
		
		void texture_2d::set_data(glm::vec<2, GLsizei> size, GLenum format, texture_data_source data)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_2D, get_id());
			glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, data.m_format, data.m_type, data.m_data);
			glBindTexture(GL_TEXTURE_2D, 0);

			die_if_error();
		}

		void texture_2d::set_sub_data(glm::vec<2, GLsizei> offset, glm::vec<2, GLsizei> size, texture_data_source data)
		{
			die_if(!is_valid());
			die_if(glm::any(glm::greaterThan(offset + size, get_size())));

			glBindTexture(GL_TEXTURE_2D, get_id());
			glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x, offset.y, size.x, size.y, data.m_format, data.m_type, data.m_data);
			glBindTexture(GL_TEXTURE_2D, 0);

			die_if_error();
		}

		glm::vec<2, GLsizei> texture_2d::get_size() const
		{
			die_if(!is_valid());

			auto size = glm::vec<2, GLsizei>();
			
			// note: glGetTexLevelParameteriv is not available in GLES :(
			glBindTexture(GL_TEXTURE_2D, get_id());
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &size.x);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &size.y);
			glBindTexture(GL_TEXTURE_2D, 0);

			die_if_error();
			return size;
		}

		void texture_2d::set_wrap_mode(GLenum mode)
		{
			set_wrap_mode(mode, mode);
		}

		void texture_2d::set_wrap_mode(GLenum x_mode, GLenum y_mode)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_2D, get_id());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, x_mode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, y_mode);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			die_if_error();
		}

		void texture_2d::set_min_filter(GLenum mode)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_2D, get_id());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			die_if_error();
		}
		
		void texture_2d::set_mag_filter(GLenum mode)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_2D, get_id());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			die_if_error();
		}

		void texture_2d::set_anisotropy(GLfloat value)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_2D, get_id());
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			die_if_error();
		}
		
		void texture_2d::generate_mipmaps()
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_2D, get_id());
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			die_if_error();
		}


		texture_cubemap::texture_cubemap()
		{
			auto id = GLuint{ 0 };
			glGenTextures(1, &id);
			die_if(!id);

			reset(id, [] (GLuint id) { glDeleteTextures(1, &id); });

			set_min_filter(GL_NEAREST);
			set_mag_filter(GL_NEAREST);
			set_wrap_mode(GL_CLAMP_TO_EDGE);
			set_anisotropy(1.f);
		}
		
		void texture_cubemap::set_data(GLenum cubemap_side, glm::vec<2, GLsizei> size, GLenum format, texture_data_source data)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_CUBE_MAP, get_id());
			glTexImage2D(cubemap_side, 0, format, size.x, size.y, 0, data.m_format, data.m_type, data.m_data);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

			die_if_error();
		}

		void texture_cubemap::set_sub_data(GLenum cubemap_side, glm::vec<2, GLsizei> offset, glm::vec<2, GLsizei> size, texture_data_source data)
		{
			die_if(!is_valid());
			die_if(glm::any(glm::greaterThan(offset + size, get_size())));

			glBindTexture(GL_TEXTURE_CUBE_MAP, get_id());
			glTexSubImage2D(cubemap_side, 0, offset.x, offset.y, size.x, size.y, data.m_format, data.m_type, data.m_data);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

			die_if_error();
		}

		glm::vec<2, GLsizei> texture_cubemap::get_size() const
		{
			die_if(!is_valid());

			auto size = glm::vec<2, GLsizei>();
			
			// note: glGetTexLevelParameteriv is not available in GLES :(
			glBindTexture(GL_TEXTURE_CUBE_MAP, get_id());
			glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_WIDTH, &size.x);
			glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_HEIGHT, &size.y);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

			die_if_error();
			return size;
		}

		void texture_cubemap::set_wrap_mode(GLenum mode)
		{
			set_wrap_mode(mode, mode, mode);
		}

		void texture_cubemap::set_wrap_mode(GLenum x_mode, GLenum y_mode, GLenum z_mode)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_CUBE_MAP, get_id());
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, x_mode);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, y_mode);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, z_mode);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			
			die_if_error();
		}

		void texture_cubemap::set_min_filter(GLenum mode)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_CUBE_MAP, get_id());
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mode);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			
			die_if_error();
		}
		
		void texture_cubemap::set_mag_filter(GLenum mode)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_CUBE_MAP, get_id());
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mode);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			
			die_if_error();
		}

		void texture_cubemap::set_anisotropy(GLfloat value)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_CUBE_MAP, get_id());
			glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			
			die_if_error();
		}
		
		void texture_cubemap::generate_mipmaps()
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_CUBE_MAP, get_id());
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			
			die_if_error();
		}

		
		texture_2d_array::texture_2d_array()
		{
			auto id = GLuint{ 0 };
			glGenTextures(1, &id);
			die_if(!id);

			reset(id, [] (GLuint id) { glDeleteTextures(1, &id); });
			
			set_min_filter(GL_NEAREST);
			set_mag_filter(GL_NEAREST);
			set_wrap_mode(GL_CLAMP_TO_EDGE);
			set_anisotropy(1.f);
		}
		
		void texture_2d_array::set_data(glm::vec<3, GLsizei> size, GLenum format, texture_data_source data)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_2D_ARRAY, get_id());
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, size.x, size.y, size.z, 0, data.m_format, data.m_type, data.m_data);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

			die_if_error();
		}
		
		void texture_2d_array::set_sub_data(glm::vec<3, GLsizei> offset, glm::vec<3, GLsizei> size, texture_data_source data)
		{
			die_if(!is_valid());
			die_if(glm::any(glm::greaterThan(offset + size, get_size())));

			glBindTexture(GL_TEXTURE_2D, get_id());
			glTexSubImage3D(GL_TEXTURE_2D, 0, offset.x, offset.y, offset.z, size.x, size.y, size.z, data.m_format, data.m_type, data.m_data);
			glBindTexture(GL_TEXTURE_2D, 0);

			die_if_error();
		}

		glm::vec<3, GLsizei> texture_2d_array::get_size() const
		{
			die_if(!is_valid());

			auto size = glm::vec<3, GLsizei>();
			
			// note: glGetTexLevelParameteriv is not available in GLES :(
			glBindTexture(GL_TEXTURE_2D_ARRAY, get_id());
			glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_WIDTH, &size.x);
			glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_HEIGHT, &size.y);
			glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_DEPTH, &size.z);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

			die_if_error();
			return size;
		}

		void texture_2d_array::set_wrap_mode(GLenum mode)
		{
			set_wrap_mode(mode, mode, mode);
		}

		void texture_2d_array::set_wrap_mode(GLenum x_mode, GLenum y_mode, GLenum z_mode)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_2D_ARRAY, get_id());
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, x_mode);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, y_mode);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, z_mode);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			
			die_if_error();
		}

		void texture_2d_array::set_min_filter(GLenum mode)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_2D_ARRAY, get_id());
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, mode);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			
			die_if_error();
		}
		
		void texture_2d_array::set_mag_filter(GLenum mode)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_2D_ARRAY, get_id());
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, mode);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			
			die_if_error();
		}

		void texture_2d_array::set_anisotropy(GLfloat value)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_2D_ARRAY, get_id());
			glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			
			die_if_error();
		}
		
		void texture_2d_array::generate_mipmaps()
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_2D_ARRAY, get_id());
			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			
			die_if_error();
		}
		
		texture_3d::texture_3d()
		{
			auto id = GLuint{ 0 };
			glGenTextures(1, &id);
			die_if(!id);

			reset(id, [] (GLuint id) { glDeleteTextures(1, &id); });
			
			set_min_filter(GL_NEAREST);
			set_mag_filter(GL_NEAREST);
			set_wrap_mode(GL_CLAMP_TO_EDGE);
			set_anisotropy(1.f);
		}
		
		void texture_3d::set_data(glm::vec<3, GLsizei> size, GLenum format, texture_data_source data)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_3D, get_id());
			glTexImage3D(GL_TEXTURE_3D, 0, format, size.x, size.y, size.z, 0, data.m_format, data.m_type, data.m_data);
			glBindTexture(GL_TEXTURE_3D, 0);

			die_if_error();
		}
		
		void texture_3d::set_sub_data(glm::vec<3, GLsizei> offset, glm::vec<3, GLsizei> size, texture_data_source data)
		{
			die_if(!is_valid());
			die_if(glm::any(glm::greaterThan(offset + size, get_size())));

			glBindTexture(GL_TEXTURE_2D, get_id());
			glTexSubImage3D(GL_TEXTURE_2D, 0, offset.x, offset.y, offset.z, size.x, size.y, size.z, data.m_format, data.m_type, data.m_data);
			glBindTexture(GL_TEXTURE_2D, 0);

			die_if_error();
		}

		glm::vec<3, GLsizei> texture_3d::get_size() const
		{
			die_if(!is_valid());

			auto size = glm::vec<3, GLsizei>();
			
			// note: glGetTexLevelParameteriv is not available in GLES :(
			glBindTexture(GL_TEXTURE_3D, get_id());
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &size.x);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &size.y);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &size.z);
			glBindTexture(GL_TEXTURE_3D, 0);

			die_if_error();
			return size;
		}

		void texture_3d::set_wrap_mode(GLenum mode)
		{
			set_wrap_mode(mode, mode, mode);
		}

		void texture_3d::set_wrap_mode(GLenum x_mode, GLenum y_mode, GLenum z_mode)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_3D, get_id());
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, x_mode);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, y_mode);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, z_mode);
			glBindTexture(GL_TEXTURE_3D, 0);
			
			die_if_error();
		}

		void texture_3d::set_min_filter(GLenum mode)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_3D, get_id());
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, mode);
			glBindTexture(GL_TEXTURE_3D, 0);
			
			die_if_error();
		}
		
		void texture_3d::set_mag_filter(GLenum mode)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_3D, get_id());
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, mode);
			glBindTexture(GL_TEXTURE_3D, 0);
			
			die_if_error();
		}

		void texture_3d::set_anisotropy(GLfloat value)
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_3D, get_id());
			glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);
			glBindTexture(GL_TEXTURE_3D, 0);
			
			die_if_error();
		}
		
		void texture_3d::generate_mipmaps()
		{
			die_if(!is_valid());

			glBindTexture(GL_TEXTURE_3D, get_id());
			glGenerateMipmap(GL_TEXTURE_3D);
			glBindTexture(GL_TEXTURE_3D, 0);
			
			die_if_error();
		}
		
	} // gl
	
} // bump