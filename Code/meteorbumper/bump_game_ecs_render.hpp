#pragma once

#include "bump_gl.hpp"

#include <entt.hpp>

namespace bump
{
	
	struct mbp_model;
	class perspective_camera;
	class camera_matrices;

	namespace game::ecs
	{
		
		class render_system
		{
		public:

			render_system() = default;

			void render(entt::registry& registry, gl::renderer& renderer, perspective_camera const& scene_camera, camera_matrices const& matrices);
		};
		
		class skybox_renderable
		{
		public:

			skybox_renderable(mbp_model const& model, gl::shader_program const& shader, gl::texture_cubemap const& texture);

			void render(gl::renderer& renderer, perspective_camera const& scene_camera, camera_matrices const& matrices);

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

		class basic_renderable
		{
		public:

			explicit basic_renderable(mbp_model const& model, gl::shader_program const& shader);

			void render(gl::renderer& renderer, camera_matrices const& matrices);

			void set_transform(glm::mat4 const& transform) { m_transform = transform; }
			glm::mat4 get_transform() const { return m_transform; }

		private:

			gl::shader_program const* m_shader;
			GLint m_in_VertexPosition;
			GLint m_u_MVP;
			GLint m_u_Color;

			struct submesh_data
			{
				glm::vec3 m_color;

				gl::buffer m_vertices;
				gl::buffer m_indices;
				gl::vertex_array m_vertex_array;
			};

			std::vector<submesh_data> m_submeshes;

			glm::mat4 m_transform;
		};

	} // game::ecs
	
} // bump