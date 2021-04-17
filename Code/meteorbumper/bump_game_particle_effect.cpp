#include "bump_game_particle_effect.hpp"

#include "bump_camera.hpp"
#include "bump_transform.hpp"
#include "bump_physics.hpp"

namespace bump
{
	
	namespace game
	{
		
		particle_effect::particle_effect(entt::registry& registry, gl::shader_program const& shader):
			m_registry(registry),
			m_shader(shader),
			m_in_Position(shader.get_attribute_location("in_Position")),
			m_in_Color(shader.get_attribute_location("in_Color")),
			m_u_MVP(shader.get_uniform_location("u_MVP")),
			m_u_Size(shader.get_uniform_location("u_Size")),
			m_origin(glm::mat4(1.f)),
			m_max_lifetime(std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(2.f))),
			m_color_map{ { 0.f, glm::vec4(1.f) }, { 1.f, glm::vec4(1.f) } },
			m_spawn_radius_m(0.5f),
			m_spawning_enabled(false),
			m_spawn_period(std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(1.f / 100.f))),
			m_spawn_accumulator(0),
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

		void particle_effect::update(high_res_duration_t dt)
		{
			// update lifetimes, destroy expired particles
			{
				auto view = m_registry.view<particle_data>();

				for (auto id : view)
				{
					auto& p = view.get<particle_data>(id);
					p.m_lifetime += dt;

					if (p.m_lifetime > m_max_lifetime)
						m_registry.destroy(id);
				}
			}

			// spawn
			if (m_spawning_enabled)
			{
				m_spawn_accumulator += dt;

				while (m_spawn_accumulator >= m_spawn_period)
				{
					auto id = m_registry.create();

					auto& particle = m_registry.emplace<particle_data>(id);
					particle.m_lifetime = high_res_duration_t{ 0 };

					auto const particle_mass_kg = 0.01f; // todo: make this configurable???
					auto const particle_radius_m = 0.01f; // todo: make this configurable???

					auto& rigidbody = m_registry.emplace<physics::rigidbody>(id);
					rigidbody.set_mass(particle_mass_kg);
					rigidbody.set_local_inertia_tensor(physics::make_sphere_inertia_tensor(particle_mass_kg, particle_radius_m));

					rigidbody.set_position(get_position(m_origin)); // todo: get position from spawn function

					auto const d = std::uniform_real_distribution<float>(-1.f, 1.f);
					auto const v = glm::vec3(d(m_rng), d(m_rng), d(m_rng)) * 5.f;
					rigidbody.set_velocity(v); // todo: get velocity from spawn function

					auto& collider = m_registry.emplace<physics::collider>(id);
					collider.set_shape({ physics::sphere_shape{ particle_radius_m } });
					collider.set_collision_layer(physics::collision_layers::PARTICLES);
					collider.set_collision_mask(physics::collision_layers::ASTEROIDS | physics::collision_layers::POWERUPS | physics::collision_layers::PLAYER);

					m_spawn_accumulator -= m_spawn_period;
				}
			}
		}

		void particle_effect::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
			auto view = m_registry.view<particle_data, physics::rigidbody>();

			if (view.empty())
				return;

			auto instance_count = view.size();

			m_frame_positions.reserve(instance_count);
			m_frame_colors.reserve(instance_count);

			for (auto id : view)
			{
				auto [p, rb] = view.get<particle_data, physics::rigidbody>(id);

				m_frame_positions.push_back(rb.get_position());
				m_frame_colors.push_back(glm::vec4(1.f)); // todo: get color from lifetime
			}

			m_instance_positions.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_positions.front()), 3, instance_count, GL_STREAM_DRAW);
			m_instance_colors.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_frame_colors.front()), 4, instance_count, GL_STREAM_DRAW);

			renderer.set_program(m_shader);
			renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::mat4(1.f)));
			renderer.set_uniform_1f(m_u_Size, 1.f); // todo: make this configurable???
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_arrays(GL_POINTS, 1, instance_count);

			renderer.clear_vertex_array();
			renderer.clear_program();

			m_frame_positions.clear();
			m_frame_colors.clear();
		}
		
	} // game
	
} // bump
