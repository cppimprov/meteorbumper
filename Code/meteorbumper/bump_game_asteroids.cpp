#include "bump_game_asteroids.hpp"

#include "bump_camera.hpp"
#include "bump_die.hpp"
#include "bump_game_player.hpp"
#include "bump_game_powerups.hpp"
#include "bump_mbp_model.hpp"
#include "bump_physics.hpp"
#include "bump_random.hpp"
#include "bump_transform.hpp"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <Tracy.hpp>

#include <algorithm>
#include <random>

namespace bump
{
	
	namespace game
	{
		
		asteroid_field::asteroid_field(entt::registry& registry, powerups& powerups, mbp_model const& model, gl::shader_program const& shader, gl::shader_program const& hit_shader):
			m_registry(registry),
			m_powerups(powerups),
			m_shader(shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_in_VertexNormal(shader.get_attribute_location("in_VertexNormal")),
			m_in_MVP(shader.get_attribute_location("in_MVP")),
			m_in_NormalMatrix(shader.get_attribute_location("in_NormalMatrix")),
			m_in_Color(shader.get_attribute_location("in_Color")),
			m_in_Scale(shader.get_attribute_location("in_Scale")),
			m_rng(std::random_device()()),
			m_wave_number(0),
			m_asteroid_type_probability{
				{ 0.40f, asteroid_type::SMALL },
				{ 0.70f, asteroid_type::MEDIUM },
				{ 1.00f, asteroid_type::LARGE } },
			m_asteroid_type_data{
				{ asteroid_type::SMALL, { 0.5f, 30.f, 200.f } },
				{ asteroid_type::MEDIUM, { 1.f, 80.f, 750.f } },
				{ asteroid_type::LARGE, { 2.f, 150.f, 2000.f } } },
			m_hit_effects(registry, hit_shader)
		{
			// setup mesh buffers
			die_if(model.m_submeshes.size() != 1);

			auto const& mesh = model.m_submeshes.front();

			m_vertices.set_data(GL_ARRAY_BUFFER, mesh.m_mesh.m_vertices.data(), 3, mesh.m_mesh.m_vertices.size() / 3, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertices);
			
			m_normals.set_data(GL_ARRAY_BUFFER, mesh.m_mesh.m_normals.data(), 3, mesh.m_mesh.m_normals.size() / 3, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexNormal, m_normals);

			m_indices.set_data(GL_ELEMENT_ARRAY_BUFFER, mesh.m_mesh.m_indices.data(), 1, mesh.m_mesh.m_indices.size(), GL_STATIC_DRAW);
			m_vertex_array.set_index_buffer(m_indices);

			// set up instance buffers
			m_transforms.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 16, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_MVP, m_transforms, 4, 4, 1);
			m_normal_matrices.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 9, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_NormalMatrix, m_normal_matrices, 3, 3, 1);
			m_colors.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Color, m_colors, 1);
			m_scales.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 1, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Scale, m_scales, 1);

			// set up collision effects
			{
				auto const color_map = std::map<float, glm::vec4>
				{
					{ 0.0f, { 0.2f, 0.2f, 0.2f, 1.f } },
					{ 0.7f, { 0.1f, 0.1f, 0.1f, 0.8f } },
					{ 0.9f, { 0.05f, 0.05f, 0.05f, 0.5f } },
					{ 1.0f, { 0.01f, 0.01f, 0.01f, 0.2f } },
				};

				auto const size_map = std::map<float, float>
				{
					{ 0.0f, 3.0f },
					{ 0.9f, 3.0f },
					{ 1.0f, 5.0f },
				};

				m_hit_effects.set_spawn_radius(0.25f);
				m_hit_effects.set_random_velocity({ 10.f, 10.f, 10.f });
				m_hit_effects.set_max_lifetime(high_res_duration_from_seconds(4.0f));
				m_hit_effects.set_max_lifetime_random(high_res_duration_from_seconds(1.f));
				m_hit_effects.set_color_update_fn(make_color_update_fn(m_hit_effects, color_map));
				m_hit_effects.set_size_update_fn(make_size_update_fn(m_hit_effects, size_map));
			}

			spawn_wave();
		}

		asteroid_field::~asteroid_field()
		{
			auto view = m_registry.view<asteroid_data>();

			for (auto id : view)
				m_registry.destroy(id);
		}

		void asteroid_field::update(high_res_duration_t dt)
		{
			auto to_destroy = std::vector<entt::entity>();
			
			struct destroyed_asteroid_data
			{
				asteroid_type m_type;
				glm::vec3 m_position;
				glm::vec3 m_velocity;
			};

			auto destroyed_data = std::vector<destroyed_asteroid_data>();

			m_registry.view<asteroid_data, physics::rigidbody>().each(
				[&] (entt::entity e, asteroid_data& data, physics::rigidbody& rigidbody)
				{
					if (data.m_hp < 0)
					{
						to_destroy.push_back(e);
						destroyed_data.push_back({ data.m_type, rigidbody.get_position(), rigidbody.get_velocity() });
					}
				});
			
			m_registry.destroy(to_destroy.begin(), to_destroy.end());

			using dist_sz = std::uniform_int_distribution<std::size_t>;

			for (auto const& destroyed : destroyed_data)
			{
				auto const mediums = (destroyed.m_type == asteroid_type::LARGE ? dist_sz(1, 2)(m_rng) : std::size_t{ 0 });
				auto const smalls =  (destroyed.m_type == asteroid_type::LARGE ? dist_sz(3, 4)(m_rng) : destroyed.m_type == asteroid_type::MEDIUM ? dist_sz(2, 3)(m_rng) : std::size_t{ 0 });
				auto const powerups = (dist_sz(0, 10)(m_rng) < 2);

				auto const base_color = glm::vec3(0.8f);
				auto const max_color_offset = glm::vec3(0.2f);

				for (auto i = std::size_t{ 0 }; i != mediums; ++i)
				{
					auto const type = asteroid_type::MEDIUM;
					auto const hp = m_asteroid_type_data.at(type).m_hp;
					auto const circle_point = random::point_in_ring_2d(m_rng, 0.3f, 0.5f) * 10.f * m_asteroid_type_data.at(destroyed.m_type).m_scale;
					auto const color = random::color_offset_rgb(m_rng, base_color, max_color_offset);
					auto const scale = random::scale(m_rng, m_asteroid_type_data.at(type).m_scale, m_asteroid_type_data.at(type).m_scale * 0.1f);
					auto const mass = m_asteroid_type_data.at(type).m_mass;
					auto const position = glm::vec3{ circle_point.x, 0.f, circle_point.y } + destroyed.m_position;
					auto const velocity_dir = glm::normalize(position - destroyed.m_position);
					auto const velocity_scale = random::scale(m_rng, 20.f, 10.f);
					auto const velocity = destroyed.m_velocity + velocity_dir * velocity_scale;

					auto spawn_data = asteroid_spawn_data
					{
						type, hp, color, scale,
						mass, position, velocity,
					};

					spawn_asteroid(spawn_data);
				}

				for (auto i = std::size_t{ 0 }; i != smalls; ++i)
				{
					auto const type = asteroid_type::SMALL;
					auto const hp = m_asteroid_type_data.at(type).m_hp;
					auto const circle_point = random::point_in_ring_2d(m_rng, 0.5f, 0.75f) * 10.f * m_asteroid_type_data.at(destroyed.m_type).m_scale;
					auto const color = random::color_offset_rgb(m_rng, base_color, max_color_offset);
					auto const scale = random::scale(m_rng, m_asteroid_type_data.at(type).m_scale, m_asteroid_type_data.at(type).m_scale * 0.1f);
					auto const mass = m_asteroid_type_data.at(type).m_mass;
					auto const position = glm::vec3{ circle_point.x, 0.f, circle_point.y } + destroyed.m_position;
					auto const velocity_dir = glm::normalize(position - destroyed.m_position);
					auto const velocity_scale = random::scale(m_rng, 35.f, 10.f);
					auto const velocity = destroyed.m_velocity + velocity_dir * velocity_scale;

					auto spawn_data = asteroid_spawn_data
					{
						type, hp, color, scale,
						mass, position, velocity,
					};

					spawn_asteroid(spawn_data);
				}

				if (powerups)
				{
					auto powerup_type_probability = std::map<float, powerups::powerup_type>
					{
						{ 0.4f, powerups::powerup_type::RESET_SHIELDS },
						{ 0.8f, powerups::powerup_type::RESET_ARMOR },
						{ 1.0f, powerups::powerup_type::UPGRADE_LASERS },
					};
					
					auto const type = random::type_from_probability_map(m_rng, powerup_type_probability);
					m_powerups.spawn(destroyed.m_position, type);
				}
			}
			
			for (auto p : m_frame_hit_positions)
			{
				auto m = glm::mat4(1.f);
				set_position(m, p);

				m_hit_effects.set_origin(m);
				m_hit_effects.spawn_once(15);
			}
			m_frame_hit_positions.clear();

			m_hit_effects.update(dt);

			if (is_wave_complete())
				spawn_wave();
		}

		void asteroid_field::render_scene(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("asteroid_field::render_scene()");

			// get instance data from components
			auto view = m_registry.view<asteroid_data, physics::rigidbody>();

			if (view.empty())
				return;

			for (auto id : view)
			{
				auto [data, physics] = view.get<asteroid_data, physics::rigidbody>(id);
				auto const transform = physics.get_transform();
				m_instance_transforms.push_back(matrices.model_view_projection_matrix(transform));
				m_instance_normal_matrices.push_back(matrices.normal_matrix(transform));
				m_instance_colors.push_back(data.m_color);
				m_instance_scales.push_back(data.m_model_scale);
			}

			// upload instance data to buffers
			m_transforms.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_instance_transforms.front()), 16, m_instance_transforms.size(), GL_STREAM_DRAW);
			m_normal_matrices.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_instance_normal_matrices.front()), 9, m_instance_normal_matrices.size(), GL_STREAM_DRAW);
			m_colors.set_data(GL_ARRAY_BUFFER, glm::value_ptr(m_instance_colors.front()), 3, m_instance_colors.size(), GL_STREAM_DRAW);
			m_scales.set_data(GL_ARRAY_BUFFER, m_instance_scales.data(), 1, m_instance_scales.size(), GL_STREAM_DRAW);

			// render
			renderer.set_program(m_shader);
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_indexed(GL_TRIANGLES, m_indices.get_element_count(), m_indices.get_component_type(), view.size());

			renderer.clear_vertex_array();
			renderer.clear_program();

			// clear instance data
			m_instance_transforms.clear();
			m_instance_normal_matrices.clear();
			m_instance_colors.clear();
			m_instance_scales.clear();
		}
		
		void asteroid_field::render_particles(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("asteroid_field::render_particles()");

			m_hit_effects.render(renderer, matrices);
		}
		
		bool asteroid_field::is_wave_complete() const
		{
			return m_registry.view<asteroid_data>().empty();
		}

		void asteroid_field::spawn_wave()
		{
			auto const max_wave_number = 20.f;
			auto const min_asteroids = 5.f;
			auto const max_asteroids = 100.f;
			auto const num_asteroids = static_cast<std::size_t>(
				glm::mix(min_asteroids, max_asteroids, glm::clamp(m_wave_number / max_wave_number, 0.f, 1.f)));

			auto const min_radius = 100.f;
			auto const max_radius = 250.f;
			
			auto const base_color = glm::vec3(0.8f);
			auto const max_color_offset = glm::vec3(0.2f);

			auto const max_target_radius = 10.f;
			auto const base_velocity = 25.f;
			auto const max_velocity_offset = 10.f;

			for (auto i = 0; i != num_asteroids; ++i)
			{
				auto const type = random::type_from_probability_map(m_rng, m_asteroid_type_probability);
				auto const hp = m_asteroid_type_data.at(type).m_hp;
				auto const circle_point = random::point_in_ring_2d(m_rng, min_radius, max_radius);
				auto const mass = m_asteroid_type_data.at(type).m_mass;
				auto const position = glm::vec3{ circle_point.x, 0.f, circle_point.y };
				auto const color = random::color_offset_rgb(m_rng, base_color, max_color_offset);
				auto const scale = random::scale(m_rng, m_asteroid_type_data.at(type).m_scale, m_asteroid_type_data.at(type).m_scale * 0.1f);

				auto const target_circle_point = random::point_in_ring_2d(m_rng, 0.f, max_target_radius);
				auto const target_position = glm::vec3{ target_circle_point.x, 0.f, target_circle_point.y };
				auto const velocity_dir = glm::normalize(target_position - position);
				auto const velocity_scale = random::scale(m_rng, base_velocity, max_velocity_offset);
				auto const velocity = velocity_dir * velocity_scale;

				auto spawn_data = asteroid_spawn_data
				{
					type, hp, color, scale,
					mass, position, velocity,
				};

				spawn_asteroid(spawn_data);
			}

			++m_wave_number;
		}

		void asteroid_field::spawn_asteroid(asteroid_spawn_data const& spawn_data)
		{
			auto id = m_registry.create();

			auto& data = m_registry.emplace<asteroid_data>(id);
			data.m_type = spawn_data.m_type;
			data.m_hp = spawn_data.m_hp;
			data.m_color = spawn_data.m_color;
			data.m_model_scale = spawn_data.m_model_scale;

			auto model_radius = 10.f * spawn_data.m_model_scale; // TODO: use a normalized sphere!!!

			auto& rigidbody = m_registry.emplace<physics::rigidbody>(id);
			rigidbody.set_mass(spawn_data.m_mass);
			rigidbody.set_local_inertia_tensor(physics::make_sphere_inertia_tensor(spawn_data.m_mass, model_radius));
			rigidbody.set_linear_factor({ 1.f, 0.f, 1.f }); // restrict movement on y axis
			rigidbody.set_angular_factor({ 0.f, 1.f, 0.f }); // restrict rotation to y axis only
			rigidbody.set_position(spawn_data.m_position);
			rigidbody.set_velocity(spawn_data.m_velocity);
			
			auto& collider = m_registry.emplace<physics::collider>(id);
			collider.set_shape({ physics::sphere_shape{ model_radius } });

			auto callback = [=] (entt::entity other, physics::collision_data const& hit, float)
			{
				if (m_registry.has<player_weapon_damage>(other))
				{
					auto& damage = m_registry.get<player_weapon_damage>(other);
					auto& data = m_registry.get<asteroid_data>(id);
					data.m_hp -= damage.m_damage;
				}

				if (!m_registry.has<particle_effect::particle_data>(other))
					m_frame_hit_positions.push_back(hit.m_point);
			};

			collider.set_callback(callback);
		}

	} // game
	
} // bump