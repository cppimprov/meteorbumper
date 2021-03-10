#pragma once

#include "bump_gl_object_handle.hpp"

#include <GL/glew.h>

#include <string>

namespace bump
{
	
	namespace gl
	{
		
		class shader_object : public object_handle
		{
		public:

			explicit shader_object(GLenum type);

			void set_source(std::string const& source);

			bool compile();
			bool is_compiled() const;

			GLenum get_type() const;
			std::string get_log() const;
		};

		class shader_program : public object_handle
		{
		public:

			shader_program();

			void attach(shader_object const& object);
			void detach(shader_object const& object);

			bool link();
			bool is_linked() const;

			bool validate();
			bool is_validated() const;

			std::string get_log() const;

			GLint get_attribute_location(std::string const& name) const;
			GLint get_uniform_location(std::string const& name) const;
		};
		
	} // gl
	
} // bump
