#pragma once

#include "bump_camera.hpp"
#include "bump_gl.hpp"

namespace bump
{
	
	struct mbp_model;

	namespace game
	{
		
		class skybox
		{
		public:

			skybox(mbp_model const& model, gl::shader_program const& shader, gl::texture_cubemap const& texture);

			void render_scene(gl::renderer& renderer, perspective_camera const& scene_camera, camera_matrices const& matrices);

		private:

			gl::shader_program const* m_shader;
			GLint m_in_VertexPosition;
			GLint m_u_MVP;
			GLint m_u_Scale;
			GLint m_u_CubemapTexture;

			gl::texture_cubemap const* m_texture;
			
			gl::buffer m_vertices;
			gl::buffer m_indices;
			gl::vertex_array m_vertex_array;
		};

	} // game
	
} // bump