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
		
		class basic_renderable
		{
		public:

			explicit basic_renderable(mbp_model const& model, gl::shader_program const& shader);

			basic_renderable(basic_renderable const&) = delete;
			basic_renderable& operator=(basic_renderable const&) = delete;
			
			basic_renderable(basic_renderable&&) = default;
			basic_renderable& operator=(basic_renderable&&) = default;

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