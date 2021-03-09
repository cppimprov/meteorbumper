#pragma once

#include "bump_gl_object_handle.hpp"
#include "bump_gl_traits.hpp"

#include <glm/glm.hpp>
#include <GL/glew.h>

namespace bump
{
	
	namespace gl
	{
		
		// helper class to specify texture constructor parameters
		class texture_data_source
		{
		public:

			GLenum m_type;
			GLenum m_format;
			void const* m_data;
		};

		// null data (blank texture)
		// infers data type from ComponentT
		template<class ComponentT>
		texture_data_source make_texture_data_source(GLenum format)
		{
			return { traits::component_type_v<ComponentT>, format, nullptr };
		}

		// null data (blank texture)
		// manually specify the data type
		inline texture_data_source make_texture_data_source(GLenum type, GLenum format)
		{
			return { type, format, nullptr };
		}

		// infers data type from ComponentT
		template<class ComponentT>
		texture_data_source make_texture_data_source(GLenum format, ComponentT const* data)
		{
			return { traits::component_type_v<ComponentT>, format, data };
		}

		// manually specify the data type (NOT inferred from ComponentT)
		// needed for (for example) depth textures, where the data type does not match the format
		template<class ComponentT>
		texture_data_source make_texture_data_source(GLenum type, GLenum format, ComponentT const* data)
		{
			return { type, format, data };
		}

		class texture_2d : public object_handle<>
		{
		public:

			texture_2d();

			void set_data(glm::vec<2, GLsizei> size, GLenum format, texture_data_source data);
			void set_sub_data(glm::vec<2, GLsizei> offset, glm::vec<2, GLsizei> size, texture_data_source data);

			glm::vec<2, GLsizei> get_size() const;

			void set_wrap_mode(GLenum mode);
			void set_wrap_mode(GLenum x_mode, GLenum y_mode);
			void set_min_filter(GLenum mode);
			void set_mag_filter(GLenum mode);
			void set_anisotropy(GLfloat value);
			
			void generate_mipmaps();
		};
		
		class texture_2d_array : public object_handle<>
		{
		public:

			texture_2d_array();

			void set_data(glm::vec<3, GLsizei> size, GLenum format, texture_data_source data);
			void set_sub_data(glm::vec<3, GLsizei> offset, glm::vec<3, GLsizei> size, texture_data_source data);

			glm::vec<3, GLsizei> get_size() const;

			void set_wrap_mode(GLenum mode);
			void set_wrap_mode(GLenum x_mode, GLenum y_mode, GLenum z_mode);
			void set_min_filter(GLenum mode);
			void set_mag_filter(GLenum mode);
			void set_anisotropy(GLfloat value);
			
			void generate_mipmaps();
		};
		
		class texture_3d : public object_handle<>
		{
		public:

			texture_3d();

			void set_data(glm::vec<3, GLsizei> size, GLenum format, texture_data_source data);
			void set_sub_data(glm::vec<3, GLsizei> offset, glm::vec<3, GLsizei> size, texture_data_source data);

			glm::vec<3, GLsizei> get_size() const;

			void set_wrap_mode(GLenum mode);
			void set_wrap_mode(GLenum x_mode, GLenum y_mode, GLenum z_mode);
			void set_min_filter(GLenum mode);
			void set_mag_filter(GLenum mode);
			void set_anisotropy(GLfloat value);
			
			void generate_mipmaps();
		};
		
	} // gl
	
} // bump