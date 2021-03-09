#include "bump_gl_shader.hpp"

#include "bump_die.hpp"
#include "bump_gl_error.hpp"
#include "bump_narrow_cast.hpp"

namespace bump
{
	
	namespace gl
	{
		
		shader_object::shader_object(GLenum type)
		{
			auto id = glCreateShader(type);
			die_if(!id);

			reset(id, [] (GLuint id) { glDeleteShader(id); });
		}

		void shader_object::set_source(std::string const& source)
		{
			die_if(!is_valid());

			auto const data = source.data();
			auto const length = narrow_cast<GLint>(source.size());
			glShaderSource(get_id(), 1, &data, &length);

			die_if_error();
		}

		bool shader_object::compile() 
		{
			die_if(!is_valid());

			glCompileShader(get_id());

			die_if_error();
			return is_compiled();
		}

		bool shader_object::is_compiled() const
		{
			die_if(!is_valid());
			
			auto status = GLint{ 0 };
			glGetShaderiv(get_id(), GL_COMPILE_STATUS, &status);

			die_if_error();
			return (status == GL_TRUE);
		}

		GLenum shader_object::get_type() const
		{
			die_if(!is_valid());

			auto type = GLint{ 0 };
			glGetShaderiv(get_id(), GL_SHADER_TYPE, &type);

			die_if_error();
			return type;
		}
		
		std::string shader_object::get_log() const
		{
			die_if(!is_valid());

			auto length = GLint{ 0 };
			glGetShaderiv(get_id(), GL_INFO_LOG_LENGTH, &length);
			die_if_error();

			die_if(length < 0);
			if (length == 0) return { };

			auto log = std::string(length, '\0');
			auto written = GLint{ 0 };
			glGetShaderInfoLog(get_id(), length, &written, log.data());
			log.resize(written);
			
			die_if_error();
			return log;
		}

		shader_program::shader_program()
		{
			auto id = glCreateProgram();
			die_if(!id);

			reset(id, [] (GLuint id) { glDeleteProgram(id); });
		}

		void shader_program::attach(shader_object const& object)
		{
			die_if(!is_valid());

			glAttachShader(get_id(), object.get_id());
			die_if_error();
		}
		
		void shader_program::detach(shader_object const& object)
		{
			die_if(!is_valid());

			glDetachShader(get_id(), object.get_id());
			die_if_error();
		}

		bool shader_program::link()
		{
			die_if(!is_valid());

			glLinkProgram(get_id());

			die_if_error();
			return is_linked();
		}

		bool shader_program::is_linked() const
		{
			die_if(!is_valid());

			auto status = GLint{ 0 };
			glGetProgramiv(get_id(), GL_LINK_STATUS, &status);

			die_if_error();
			return (status == GL_TRUE);
		}

		bool shader_program::validate()
		{
			die_if(!is_valid());

			glValidateProgram(get_id());

			die_if_error();
			return is_validated();
		}

		bool shader_program::is_validated() const
		{
			die_if(!is_valid());

			auto status = GLint{ 0 };
			glGetProgramiv(get_id(), GL_VALIDATE_STATUS, &status);
			
			die_if_error();
			return (status == GL_TRUE);
		}
		
		std::string shader_program::get_log() const
		{
			die_if(!is_valid());

			auto length = GLint{ 0 };
			glGetProgramiv(get_id(), GL_INFO_LOG_LENGTH, &length);
			die_if_error();

			die_if(length < 0);
			if (length == 0) return { };

			auto log = std::string(length, '\0');
			auto written = GLint{ 0 };
			glGetProgramInfoLog(get_id(), length, &written, log.data());
			log.resize(written);
			
			die_if_error();
			return log;
		}

		GLint shader_program::get_attribute_location(std::string const& name) const
		{
			die_if(!is_valid());
			die_if(!is_linked());

			return glGetAttribLocation(get_id(), name.c_str());
		}

		GLint shader_program::get_uniform_location(std::string const& name) const
		{
			die_if(!is_valid());
			die_if(!is_linked());

			return glGetUniformLocation(get_id(), name.c_str());
		}
		
	} // gl
	
} // bump
