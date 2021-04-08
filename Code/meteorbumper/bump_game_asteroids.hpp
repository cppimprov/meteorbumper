#pragma once

#include "bump_gl.hpp"
#include "bump_time.hpp"

#include <entt.hpp>
#include <glm/glm.hpp>

#include <random>

namespace bump
{
	
	struct mbp_model;
	class camera_matrices;

	namespace game
	{

		class power_ups;
		
		class asteroid_field
		{
		public:

			explicit asteroid_field(entt::registry& registry, power_ups& powerups, mbp_model const& model, gl::shader_program const& shader);

			void update(high_res_duration_t dt);
			void render(gl::renderer& renderer, camera_matrices const& matrices);

			enum class asteroid_type { LARGE, MEDIUM, SMALL };

			struct asteroid_data
			{
				asteroid_type m_type = asteroid_type::SMALL;
				float m_hp = 0;
				glm::vec3 m_color = glm::vec3(1.f);
				float m_model_scale = 1.f;
			};

		private:

			bool is_wave_complete() const;
			void spawn_wave();
			
			struct asteroid_spawn_data
			{
				asteroid_type m_type;
				float m_hp;
				glm::vec3 m_color;
				float m_model_scale;

				float m_mass;
				glm::vec3 m_position;
				glm::vec3 m_velocity;
			};

			void spawn_asteroid(asteroid_spawn_data const& data);

			entt::registry& m_registry;
			power_ups& m_powerups;

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

			struct asteroid_type_data
			{
				float m_scale;
				float m_hp;
				float m_mass;
			};

			std::mt19937_64 m_rng;
			
			std::size_t m_wave_number;
			std::map<float, asteroid_type> m_asteroid_type_probability;
			std::map<asteroid_type, asteroid_type_data> m_asteroid_type_data;
		};

	} // game
	
} // bump
