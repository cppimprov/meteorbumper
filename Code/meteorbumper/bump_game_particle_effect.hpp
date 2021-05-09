#pragma once

#include "bump_color_map.hpp"
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
				high_res_duration_t m_lifetime = high_res_duration_from_seconds(2.f);
				glm::vec4 m_color = glm::vec4(1.f);
				float m_size = 1.f;
			};

			explicit particle_effect(entt::registry& registry, gl::shader_program const& shader);
			~particle_effect();

			void set_origin(glm::mat4 origin_transform) { m_origin = origin_transform; }
			glm::mat4 get_origin() const { return m_origin; }

			void set_max_lifetime(high_res_duration_t time) { m_max_lifetime = time; }
			high_res_duration_t get_max_lifetime() const { return m_max_lifetime; }

			void set_max_lifetime_random(high_res_duration_t time) { m_max_lifetime_random = time; }
			high_res_duration_t get_max_lifetime_random() const { return m_max_lifetime_random; }

			using color_update_fn_t = std::function<glm::vec4(entt::entity, particle_data const&)>;
			void set_color_update_fn(color_update_fn_t fn) { m_color_update_fn = fn; }
			using size_update_fn_t = std::function<float(entt::entity, particle_data const&)>;
			void set_size_update_fn(size_update_fn_t fn) { m_size_update_fn = fn; }

			void set_collision_mask(std::uint32_t mask) { m_collision_mask = mask; }
			std::uint32_t get_collision_mask() const { return m_collision_mask; }

			void set_blend_mode(gl::renderer::blending mode) { m_blend_mode = mode; }
			gl::renderer::blending get_blend_mode() const { return m_blend_mode; }

			void set_base_velocity(glm::vec3 velocity) { m_base_velocity = velocity; }
			glm::vec3 get_base_velocity() const { return m_base_velocity; }

			void set_random_velocity(glm::vec3 velocity) { m_random_velocity = velocity; }
			glm::vec3 get_random_velocity() const { return m_random_velocity; }
			
			void set_spawn_radius(float radius) { m_spawn_radius_m = radius; }
			float get_spawn_radius() const { return m_spawn_radius_m; }

			void set_spawn_enabled(bool enabled) { m_spawning_enabled = enabled; }
			bool get_spawn_enabled() const { return m_spawning_enabled; }

			void set_spawn_period(high_res_duration_t period) { m_spawn_period = period; }
			high_res_duration_t get_spawn_period() const { return m_spawn_period; }

			void set_max_particle_count(std::size_t particle_count) { m_max_particle_count = particle_count; }

			std::size_t get_size() { return m_particles.size(); }
			bool is_empty() { return m_particles.empty(); }
			void clear();

			void spawn_once(std::size_t particle_count);

			void update(high_res_duration_t dt);
			void render(gl::renderer& renderer, camera_matrices const& matrices);
			
		private:

			void spawn_particle();

			entt::registry& m_registry;

			gl::shader_program const& m_shader;
			GLint m_in_Position;
			GLint m_in_Color;
			GLint m_in_Size;
			GLint m_u_MVP;

			gl::buffer m_instance_positions;
			gl::buffer m_instance_colors;
			gl::buffer m_instance_sizes;
			gl::vertex_array m_vertex_array;

			glm::mat4 m_origin;
			high_res_duration_t m_max_lifetime;
			high_res_duration_t m_max_lifetime_random;
			std::uint32_t m_collision_mask;
			gl::renderer::blending m_blend_mode;

			glm::vec3 m_base_velocity; // in local space
			glm::vec3 m_random_velocity; // in local space
			float m_spawn_radius_m;
			bool m_spawning_enabled;
			high_res_duration_t m_spawn_period;
			high_res_duration_t m_spawn_accumulator;
			
			color_update_fn_t m_color_update_fn;
			size_update_fn_t m_size_update_fn;

			std::size_t m_max_particle_count;
			std::vector<entt::entity> m_particles;
			
			std::vector<glm::vec3> m_frame_positions;
			std::vector<glm::vec4> m_frame_colors;
			std::vector<float> m_frame_sizes;

			std::mt19937 m_rng;
		};

		inline particle_effect::color_update_fn_t make_color_update_fn(particle_effect const& effect, std::map<float, glm::vec4> color_map)
		{
			return [&, color_map] (entt::entity, particle_effect::particle_data const& p)
			{
				auto a = std::clamp(high_res_duration_to_seconds(p.m_lifetime) / high_res_duration_to_seconds(effect.get_max_lifetime()), 0.f, 1.f);
				return get_color_from_map(color_map, a);
			};
		}

		inline particle_effect::size_update_fn_t make_size_update_fn(particle_effect const& effect, std::map<float, float> size_map)
		{
			return [&, size_map] (entt::entity, particle_effect::particle_data const& p)
			{
				auto a = std::clamp(high_res_duration_to_seconds(p.m_lifetime) / high_res_duration_to_seconds(effect.get_max_lifetime()), 0.f, 1.f);
				return get_size_from_map(size_map, a);
			};
		}
		
	} // game
	
} // bump
