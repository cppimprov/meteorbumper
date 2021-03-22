#pragma once

#include "bump_gl.hpp"

#include <entt.hpp>
#include <glm/glm.hpp>

namespace bump
{
	
	struct mbp_model;
	class camera_matrices;

	namespace game
	{
		
		namespace ecs
		{

			class asteroid_data
			{
			public:

				glm::mat4 m_transform;
				glm::vec3 m_color;
			};

		} // ecs

		class asteroid_field
		{
		public:

			explicit asteroid_field(entt::registry& registry, mbp_model const& model, gl::shader_program const& shader);

			void update(entt::registry& registry);

			void render(entt::registry& registry, gl::renderer& renderer, camera_matrices const& matrices);

		private:

			gl::shader_program const* m_shader;

			GLint m_in_VertexPosition;
			GLint m_in_MVP;
			GLint m_in_Color;
			
			gl::buffer m_vertices;
			gl::buffer m_indices;
			gl::buffer m_transforms;
			gl::buffer m_colors;
			gl::vertex_array m_vertex_array;

			std::vector<glm::mat4> m_instance_transforms;
			std::vector<glm::vec3> m_instance_colors;
		};

	} // game
	
} // bump
