#pragma once

#include "bump_camera.hpp"
#include "bump_gl.hpp"

#include <glm/gtx/std_based_type.hpp>

namespace bump
{
	
	class gbuffers
	{
	public:

		explicit gbuffers(std::size_t buffer_count, glm::ivec2 screen_size);

		void recreate(std::size_t buffer_count, glm::ivec2 screen_size);

		gl::framebuffer m_framebuffer;
		std::vector<gl::texture_2d> m_buffers;
		gl::texture_2d m_depth_stencil;
	};

	class lighting_rendertarget
	{
	public:

		explicit lighting_rendertarget(glm::ivec2 screen_size);

		gl::framebuffer m_framebuffer;
		gl::texture_2d m_target;
	};

	class textured_quad
	{
	public:

		explicit textured_quad(gl::shader_program const& shader);

		glm::vec2 m_position;
		glm::vec2 m_size;

		void render(gl::texture_2d const& texture, gl::renderer& renderer, camera_matrices const& ui_matrices);

	private:

		gl::shader_program const& m_shader;
		GLint m_in_VertexPosition;
		GLint m_u_MVP;
		GLint m_u_Position;
		GLint m_u_Size;
		GLint m_u_Texture;

		gl::buffer m_vertex_buffer;
		gl::vertex_array m_vertex_array;
	};
	
} // bump
