#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

namespace bump
{
	
	namespace gl
	{

		// struct uniform_render_data
		// {
		// 	GLint m_location;
		// 	std::variant<...all_uniform_types...> m_data; // ???
		// };

		// struct texture_render_data
		// {
		// 	GLint m_location;
		// 	GLenum m_target;
		// 	GLuint m_id;
		// };

		// struct array_mesh_render_data
		// {
		// 	GLuint m_program_id; // must not be 0
		// 	std::vector<uniform_render_data> m_uniforms;
		// 	std::vector<texture_render_data> m_textures;

		// 	GLuint m_vertex_array_id; // must not be 0

		// 	GLenum m_primitive_type;
		// 	GLsizei m_vertex_count;
		// 	GLsizei m_instance_count;
		// };

		// struct indexed_mesh_render_data
		// {
		// 	GLuint m_program_id; // must not be 0
		// 	std::vector<uniform_render_data> m_uniforms;
		// 	std::vector<texture_render_data> m_textures;

		// 	GLuint m_vertex_array_id; // must not be 0

		// 	GLenum m_primitive_type;
		// 	GLsizei m_index_count;
		// 	GLenum m_index_type;
		// 	GLsizei m_instance_count;
		// };

		class renderer
		{
		public:

			void clear_color_buffers(glm::f32vec4 color = { 0.f, 0.f, 0.f, 0.f });
			void clear_depth_buffers(float depth = 1.f);
			
			// ...
		};

	} // gl
	
} // bump
