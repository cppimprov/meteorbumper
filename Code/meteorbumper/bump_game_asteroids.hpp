#pragma once

#include "bump_game_particle_effect.hpp"
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

		class powerups;
		
		class asteroid_field
		{
		public:

			explicit asteroid_field(entt::registry& registry, powerups& powerups, mbp_model const& model, gl::shader_program const& depth_shader, gl::shader_program const& shader, gl::shader_program const& hit_shader);
			~asteroid_field();

			void update(high_res_duration_t dt);
			void render_depth(gl::renderer& renderer, camera_matrices const& matrices);
			void render_scene(gl::renderer& renderer, camera_matrices const& matrices);
			void render_particles(gl::renderer& renderer, camera_matrices const& matrices);

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
			powerups& m_powerups;

			// depth stuff!
			gl::shader_program const& m_depth_shader;

			GLint m_depth_in_VertexPosition;
			GLint m_depth_in_VertexNormal;
			GLint m_depth_in_MVP;
			GLint m_depth_in_NormalMatrix;
			GLint m_depth_in_Color;
			GLint m_depth_in_Scale;
			
			gl::buffer m_depth_vertices;
			gl::buffer m_depth_normals;
			gl::buffer m_depth_indices;
			gl::buffer m_depth_transforms;
			gl::buffer m_depth_normal_matrices;
			gl::buffer m_depth_colors;
			gl::buffer m_depth_scales;
			gl::vertex_array m_depth_vertex_array;

			std::vector<glm::mat4> m_depth_instance_transforms;
			std::vector<glm::mat3> m_depth_instance_normal_matrices;
			std::vector<glm::vec3> m_depth_instance_colors;
			std::vector<float> m_depth_instance_scales;

			// non-depth stuff!
			gl::shader_program const& m_shader;

			GLint m_in_VertexPosition;
			GLint m_in_VertexNormal;
			GLint m_in_MVP;
			GLint m_in_NormalMatrix;
			GLint m_in_Color;
			GLint m_in_Scale;
			
			gl::buffer m_vertices;
			gl::buffer m_normals;
			gl::buffer m_indices;
			gl::buffer m_transforms;
			gl::buffer m_normal_matrices;
			gl::buffer m_colors;
			gl::buffer m_scales;
			gl::vertex_array m_vertex_array;

			std::vector<glm::mat4> m_instance_transforms;
			std::vector<glm::mat3> m_instance_normal_matrices;
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

			particle_effect m_hit_effects;
			std::vector<glm::vec3> m_frame_hit_positions;
		};

	} // game
	
} // bump
