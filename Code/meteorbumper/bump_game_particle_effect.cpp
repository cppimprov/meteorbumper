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
		
		particle_effect::particle_effect(entt::registry& registry, gl::shader_program const& shader):
			m_registry(registry),
			m_shader(shader),
			m_in_Position(shader.get_attribute_location("in_Position")),
			m_in_Color(shader.get_attribute_location("in_Color")),
			m_in_Size(shader.get_attribute_location("in_Size")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_LightViewProjMatrix(shader.get_uniform_location("u_LightViewProjMatrix")),
			m_u_Shadows(shader.get_uniform_location("u_Shadows")),
			m_u_EnableShadows(shader.get_uniform_location("u_EnableShadows")),
			m_origin(glm::mat4(1.f)),
			m_max_lifetime(high_res_duration_from_seconds(2.f)),
			m_max_lifetime_random(high_res_duration_from_seconds(0.f)),
			m_collision_mask(physics::collision_layers::ASTEROIDS | physics::collision_layers::POWERUPS | physics::collision_layers::PLAYER),
			m_blend_mode(gl::renderer::blending::ADD),
			m_shadows_enabled(false),
			m_base_velocity{ 0.f, 0.f, 0.f },
			m_random_velocity{ 5.f, 5.f, 5.f },
			m_spawn_radius_m(0.5f),
			m_spawning_enabled(false),
			m_spawn_period(high_res_duration_from_seconds(1.f / 100.f)),
			m_spawn_accumulator(0),
			m_color_update_fn(),
			m_size_update_fn(),
			m_max_particle_count(500u),
			m_rng(std::random_device()())
		{
			m_instance_positions.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Position, m_instance_positions, 1);
			m_instance_colors.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 4, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Color, m_instance_colors, 1);
			m_instance_sizes.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 1, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Size, m_instance_sizes, 1);
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
			// update particle data:
			{
				auto view = m_registry.view<particle_data>();

				for (auto id : m_particles)
				{
					auto& p = view.get<particle_data>(id);
					p.m_lifetime += dt;
					p.m_color = m_color_update_fn ? m_color_update_fn(id, p) : p.m_color;
					p.m_size = m_size_update_fn ? m_size_update_fn(id, p) : p.m_size;
				}

				// destroy expired particles:
				auto first_dead_particle = std::remove_if(m_particles.begin(), m_particles.end(),
					[&] (entt::entity id)
					{
						auto const& p = view.get<particle_data>(id);

						auto result = (p.m_lifetime > m_max_lifetime);

						if (result)
							m_registry.destroy(id);
						
						return result;
					});
				
				m_particles.erase(first_dead_particle, m_particles.end());
			}

			// spawn new particles
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

		void particle_effect::render(gl::renderer& renderer, camera_matrices const& light_matrices, camera_matrices const& matrices, gl::texture_2d const& shadow_map)
		{
			ZoneScopedN("particle_effect::render()");

			if (m_particles.empty())
				return;

			auto instance_count = m_particles.size();

			{
				ZoneScopedN("particle_effect::render() - get data");

				m_frame_positions.reserve(instance_count);
				m_frame_colors.reserve(instance_count);
				m_frame_sizes.reserve(instance_count);

				auto view = m_registry.view<particle_data, physics::rigidbody>();

				for (auto id : m_particles)
				{
					auto [p, rb] = view.get<particle_data, physics::rigidbody>(id);
					m_frame_positions.push_back(rb.get_position());
					m_frame_colors.push_back(p.m_color);
					m_frame_sizes.push_back(p.m_size);
				}
				
				m_instance_positions.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_positions.front()), 3, instance_count, GL_STREAM_DRAW);
				m_instance_colors.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_colors.front()), 4, instance_count, GL_STREAM_DRAW);
				m_instance_sizes.set_data(GL_ARRAY_BUFFER, m_frame_sizes.data(), 1, instance_count, GL_STREAM_DRAW);

				m_frame_positions.clear();
				m_frame_colors.clear();
				m_frame_sizes.clear();
			}
			
			{
				ZoneScopedN("particle_effect::render() - render");

				renderer.set_depth_write(gl::renderer::depth_write::DISABLED);
				renderer.set_blending(m_blend_mode);

				renderer.set_program(m_shader);
				renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::mat4(1.f)));
				renderer.set_uniform_4x4f(m_u_LightViewProjMatrix, light_matrices.m_view_projection);
				renderer.set_uniform_1i(m_u_Shadows, 0);
				renderer.set_texture_2d(0, shadow_map);
				renderer.set_uniform_1f(m_u_EnableShadows, m_shadows_enabled ? 1.f : 0.f);
				renderer.set_vertex_array(m_vertex_array);

				renderer.draw_arrays(GL_POINTS, 1, instance_count);

				renderer.clear_texture_2d(0);
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

			auto const particle_mass_kg = 0.005f;
			auto const particle_radius_m = 0.01f;

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
			collider.set_collision_mask(m_collision_mask);

			auto& particle = m_registry.emplace<particle_data>(id);
			particle.m_lifetime = l;
			particle.m_color = m_color_update_fn ? m_color_update_fn(id, particle) : glm::vec4(1.f);
			particle.m_size = m_size_update_fn ? m_size_update_fn(id, particle) : 1.f;

			m_particles.push_back(id);
		}
		
	} // game
	
} // bump
