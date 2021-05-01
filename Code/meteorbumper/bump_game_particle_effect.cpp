#include "bump_game_particle_effect.hpp"

#include "bump_camera.hpp"
#include "bump_random.hpp"
#include "bump_transform.hpp"
#include "bump_physics.hpp"

#include <Tracy.hpp>

namespace bump
{
	
	namespace game
	{
		
		namespace
		{

			glm::vec4 catmull_rom(glm::vec4 p0, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, float t)
			{
				return 0.5f * (
					(2.0f * p1) +
					(-p0 + p2) * t +
					(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
					(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t);
			}
			
			glm::vec4 get_color_from_map(std::map<float, glm::vec4> const& color_map, float a)
			{
				if (color_map.empty())
					return glm::vec4(1.f);
				
				if (color_map.size() == 1)
					return color_map.begin()->second;
				
				auto upper = color_map.upper_bound(a);
				
				if (upper == color_map.end())
					return std::prev(color_map.end())->second;
				
				auto p2 = upper;
				auto p3 = (std::next(p2) == color_map.end() ? p2 : std::next(p2));
				
				if (upper == color_map.begin())
					return upper->second;
				
				auto p1 = std::prev(upper);
				auto p0 = (p1 == color_map.begin() ? p1 : std::prev(p1));
				
				die_if(a < p1->first || a >= p2->first);
				auto t = (a - p1->first) / (p2->first - p1->first);
				
				return catmull_rom(p0->second, p1->second, p2->second, p3->second, t);
			}


		} // unnamed

		particle_effect::particle_effect(entt::registry& registry, gl::shader_program const& shader):
			m_registry(registry),
			m_shader(shader),
			m_in_Position(shader.get_attribute_location("in_Position")),
			m_in_Color(shader.get_attribute_location("in_Color")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_Size(shader.get_uniform_location("u_Size")),
			m_origin(glm::mat4(1.f)),
			m_max_lifetime(high_res_duration_from_seconds(2.f)),
			m_max_lifetime_random(high_res_duration_from_seconds(0.f)),
			m_color_map(),
			m_base_velocity{ 0.f, 0.f, 0.f },
			m_random_velocity{ 5.f, 5.f, 5.f },
			m_spawn_radius_m(0.5f),
			m_spawning_enabled(false),
			m_spawn_period(high_res_duration_from_seconds(1.f / 100.f)),
			m_spawn_accumulator(0),
			m_max_particle_count(500u),
			m_rng(std::random_device()())
		{
			m_instance_positions.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Position, m_instance_positions, 1);
			m_instance_colors.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 4, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Color, m_instance_colors, 1);
		}

		particle_effect::~particle_effect()
		{
			clear();
		}

		void particle_effect::clear()
		{
			for (auto id : m_particles)
				m_registry.destroy(id);
		}

		void particle_effect::spawn_once(std::size_t particle_count)
		{
			for (auto i = std::size_t{ 0 }; i != particle_count; ++i)
				spawn_particle();
		}

		void particle_effect::update(high_res_duration_t dt)
		{
			// update lifetimes, destroy expired particles
			{
				auto view = m_registry.view<particle_data>();

				auto first_dead_particle = std::remove_if(m_particles.begin(), m_particles.end(),
					[&] (entt::entity id)
					{
						auto& p = view.get<particle_data>(id);
						p.m_lifetime += dt;

						auto result = (p.m_lifetime > m_max_lifetime);

						if (result)
							m_registry.destroy(id);
						
						return result;
					});
				
				m_particles.erase(first_dead_particle, m_particles.end());
			}

			// spawn
			if (m_spawning_enabled)
			{
				m_spawn_accumulator += dt;

				while (m_spawn_accumulator >= m_spawn_period)
				{
					spawn_particle();
					m_spawn_accumulator -= m_spawn_period;
				}
			}
		}

		void particle_effect::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("particle_effect::render()");

			if (m_particles.empty())
				return;

			auto instance_count = m_particles.size();

			{
				ZoneScopedN("particle_effect::render() - get data");

				m_frame_positions.reserve(instance_count);
				m_frame_colors.reserve(instance_count);

				auto view = m_registry.view<particle_data, physics::rigidbody>();

				for (auto id : m_particles)
				{
					auto [p, rb] = view.get<particle_data, physics::rigidbody>(id);
					auto const lf = std::clamp(high_res_duration_to_seconds(p.m_lifetime) / high_res_duration_to_seconds(p.m_max_lifetime), 0.f, 1.f);

					m_frame_positions.push_back(rb.get_position());
					m_frame_colors.push_back(get_color_from_map(m_color_map, lf));
				}
				
				m_instance_positions.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_positions.front()), 3, instance_count, GL_STREAM_DRAW);
				m_instance_colors.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_colors.front()), 4, instance_count, GL_STREAM_DRAW);

				m_frame_positions.clear();
				m_frame_colors.clear();
			}
			
			{
				ZoneScopedN("particle_effect::render() - render");

				renderer.set_depth_write(gl::renderer::depth_write::DISABLED);
				renderer.set_blending(gl::renderer::blending::ADD);

				renderer.set_program(m_shader);
				renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::mat4(1.f)));
				renderer.set_uniform_1f(m_u_Size, 1.f); // todo: make this configurable?
				renderer.set_vertex_array(m_vertex_array);

				renderer.draw_arrays(GL_POINTS, 1, instance_count);

				renderer.clear_vertex_array();
				renderer.clear_program();

				renderer.set_blending(gl::renderer::blending::NONE);
				renderer.set_depth_write(gl::renderer::depth_write::ENABLED);
			}
		}

		void particle_effect::spawn_particle()
		{
			if (get_size() >= m_max_particle_count)
				return;

			auto id = m_registry.create();

			auto const dl = std::uniform_real_distribution<float>(0.f, 1.f);
			auto const l = high_res_duration_from_seconds(high_res_duration_to_seconds(m_max_lifetime_random) * dl(m_rng));

			auto& particle = m_registry.emplace<particle_data>(id);
			particle.m_lifetime = l;
			particle.m_max_lifetime = m_max_lifetime;

			auto const particle_mass_kg = 0.005f; // todo: make this configurable?
			auto const particle_radius_m = 0.01f; // todo: make this configurable?

			auto const p = random::point_in_ring_3d(m_rng, 0.f, m_spawn_radius_m);
			auto const d = std::uniform_real_distribution<float>(-1.f, 1.f);
			auto const v = m_base_velocity + m_random_velocity * random::point_in_ring_3d(m_rng, 0.f, 1.f);

			auto& rigidbody = m_registry.emplace<physics::rigidbody>(id);
			rigidbody.set_mass(particle_mass_kg);
			rigidbody.set_local_inertia_tensor(physics::make_sphere_inertia_tensor(particle_mass_kg, particle_radius_m));
			rigidbody.set_position(transform_point_to_world(m_origin, p));
			rigidbody.set_velocity(transform_vector_to_world(m_origin, v));

			auto& collider = m_registry.emplace<physics::collider>(id);
			collider.set_shape({ physics::sphere_shape{ particle_radius_m } });
			collider.set_collision_layer(physics::collision_layers::PARTICLES);
			collider.set_collision_mask(physics::collision_layers::ASTEROIDS | physics::collision_layers::POWERUPS | physics::collision_layers::PLAYER);

			m_particles.push_back(id);
		}
		
	} // game
	
} // bump
