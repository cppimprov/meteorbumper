#pragma once

#include "bump_gl.hpp"
#include "bump_time.hpp"

#include <entt.hpp>
#include <glm/glm.hpp>

namespace bump
{
	
	struct mbp_model;
	class camera_matrices;

	namespace game
	{
		
		class asteroid_field
		{
		public:

			explicit asteroid_field(entt::registry& registry, mbp_model const& model, gl::shader_program const& shader);

			void update(high_res_duration_t dt);
			void render(gl::renderer& renderer, camera_matrices const& matrices);

		private:

			entt::registry& m_registry;
			gl::shader_program const& m_shader;

			GLint m_in_VertexPosition;
			GLint m_in_MVP;
			GLint m_in_Color;
			GLint m_in_Scale;
			
			gl::buffer m_vertices;
			gl::buffer m_indices;
			gl::buffer m_transforms;
			gl::buffer m_colors;
			gl::buffer m_scales;
			gl::vertex_array m_vertex_array;

			std::vector<glm::mat4> m_instance_transforms;
			std::vector<glm::vec3> m_instance_colors;
			std::vector<float> m_instance_scales;

			enum class asteroid_type { LARGE, MEDIUM, SMALL };

			struct asteroid_data
			{
				asteroid_type m_type = asteroid_type::SMALL;
				float m_hp = 0;
				glm::vec3 m_color = glm::vec3(1.f);
				float m_model_scale = 1.f;
			};
		};

	} // game
	
} // bump
