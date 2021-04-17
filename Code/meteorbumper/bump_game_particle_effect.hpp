#pragma once

#include "bump_gl.hpp"
#include "bump_time.hpp"

#include <entt.hpp>
#include <glm/glm.hpp>

#include <map>
#include <random>
#include <vector>

namespace bump
{

	class camera_matrices;
	
	namespace game
	{
		
		class particle_effect
		{
		public:

			struct particle_data
			{
				high_res_duration_t m_lifetime;
			};

			explicit particle_effect(entt::registry& registry, gl::shader_program const& shader);
			~particle_effect();

			void set_origin(glm::mat4 origin_transform) { m_origin = origin_transform; }
			glm::mat4 get_origin() const { return m_origin; }

			void set_max_lifetime(high_res_duration_t time) { m_max_lifetime = time; }
			high_res_duration_t get_max_lifetime() const { return m_max_lifetime; }

			void set_color_map(std::map<float, glm::vec4> colors) { m_color_map = colors; }
			std::map<float, glm::vec4> get_color_map() const { return m_color_map; }

			// todo: add std::function<glm::vec3(rng_t&)> to get spawn position
			// todo: add a similar function to get initial direction
			
			void set_spawn_radius(float radius) { m_spawn_radius_m = radius; }
			float get_spawn_radius() const { return m_spawn_radius_m; }

			void enable_spawning() { m_spawning_enabled = true; }
			void disable_spawning() { m_spawning_enabled = false; }

			std::size_t get_size() { return m_particles.size(); }
			bool is_empty() { return m_particles.empty(); }
			void clear();

			void update(high_res_duration_t dt);
			void render(gl::renderer& renderer, camera_matrices const& matrices);
			
		private:

			entt::registry& m_registry;

			gl::shader_program const& m_shader;
			GLint m_in_Position;
			GLint m_in_Color;
			GLint m_u_MVP;
			GLint m_u_Size;

			gl::buffer m_instance_positions;
			gl::buffer m_instance_colors;
			gl::vertex_array m_vertex_array;

			glm::mat4 m_origin;
			high_res_duration_t m_max_lifetime;
			std::map<float, glm::vec4> m_color_map;

			float m_spawn_radius_m;
			bool m_spawning_enabled;
			high_res_duration_t m_spawn_period;
			high_res_duration_t m_spawn_accumulator;

			std::vector<entt::entity> m_particles;
			
			std::vector<glm::vec3> m_frame_positions;
			std::vector<glm::vec4> m_frame_colors;

			std::mt19937 m_rng;
		};
		
	} // game
	
} // bump
