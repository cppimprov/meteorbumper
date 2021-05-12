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
#include <iostream>
#include <random>

namespace bump
{
	
	namespace game
	{

		asteroid_renderable::asteroid_renderable(mbp_model const& model, gl::shader_program const& depth_shader, gl::shader_program const& shader):
			m_depth_shader(depth_shader),
			m_depth_in_VertexPosition(depth_shader.get_attribute_location("in_VertexPosition")),
			m_depth_in_MVP(depth_shader.get_attribute_location("in_MVP")),
			m_depth_in_Scale(depth_shader.get_attribute_location("in_Scale")),
			m_shader(shader),
			m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
			m_in_VertexNormal(shader.get_attribute_location("in_VertexNormal")),
			m_in_MVP(shader.get_attribute_location("in_MVP")),
			m_in_NormalMatrix(shader.get_attribute_location("in_NormalMatrix")),
			m_in_Color(shader.get_attribute_location("in_Color")),
			m_in_Scale(shader.get_attribute_location("in_Scale"))
		{
			// setup mesh buffers
			die_if(model.m_submeshes.size() != 1);

			auto const& mesh = model.m_submeshes.front();

			// set up vertex buffers
			m_vertices.set_data(GL_ARRAY_BUFFER, mesh.m_mesh.m_vertices.data(), 3, mesh.m_mesh.m_vertices.size() / 3, GL_STATIC_DRAW);
			m_depth_vertex_array.set_array_buffer(m_depth_in_VertexPosition, m_vertices);
			m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertices);
			m_normals.set_data(GL_ARRAY_BUFFER, mesh.m_mesh.m_normals.data(), 3, mesh.m_mesh.m_normals.size() / 3, GL_STATIC_DRAW);
			m_vertex_array.set_array_buffer(m_in_VertexNormal, m_normals);
			m_indices.set_data(GL_ELEMENT_ARRAY_BUFFER, mesh.m_mesh.m_indices.data(), 1, mesh.m_mesh.m_indices.size(), GL_STATIC_DRAW);
			m_depth_vertex_array.set_index_buffer(m_indices);
			m_vertex_array.set_index_buffer(m_indices);

			// set up instance buffers
			m_transforms.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 16, 0, GL_STREAM_DRAW);
			m_depth_vertex_array.set_array_buffer(m_depth_in_MVP, m_transforms, 4, 4, 1);
			m_vertex_array.set_array_buffer(m_in_MVP, m_transforms, 4, 4, 1);
			m_normal_matrices.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 9, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_NormalMatrix, m_normal_matrices, 3, 3, 1);
			m_colors.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
			m_vertex_array.set_array_buffer(m_in_Color, m_colors, 1);
			m_scales.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 1, 0, GL_STREAM_DRAW);
			m_depth_vertex_array.set_array_buffer(m_depth_in_Scale, m_scales, 1);
			m_vertex_array.set_array_buffer(m_in_Scale, m_scales, 1);
		}

		void asteroid_renderable::render_depth(gl::renderer& renderer, camera_matrices const&, std::vector<glm::mat4> const& transforms, std::vector<float> const& scales)
		{
			// upload instance data to buffers
			m_transforms.set_data(GL_ARRAY_BUFFER, glm::value_ptr(transforms.front()), 16, transforms.size(), GL_STREAM_DRAW);
			m_scales.set_data(GL_ARRAY_BUFFER, scales.data(), 1, scales.size(), GL_STREAM_DRAW);

			// render
			renderer.set_program(m_depth_shader);
			renderer.set_vertex_array(m_depth_vertex_array);

			renderer.draw_indexed(GL_TRIANGLES, m_indices.get_element_count(), m_indices.get_component_type(), transforms.size());

			renderer.clear_vertex_array();
			renderer.clear_program();
		}

		void asteroid_renderable::render_scene(gl::renderer& renderer, camera_matrices const&, std::vector<glm::mat4> const& transforms, std::vector<glm::mat3> const& normal_matrices, std::vector<glm::vec3> const& colors, std::vector<float> const& scales)
		{
			// upload instance data to buffers
			m_transforms.set_data(GL_ARRAY_BUFFER, glm::value_ptr(transforms.front()), 16, transforms.size(), GL_STREAM_DRAW);
			m_normal_matrices.set_data(GL_ARRAY_BUFFER, glm::value_ptr(normal_matrices.front()), 9, normal_matrices.size(), GL_STREAM_DRAW);
			m_colors.set_data(GL_ARRAY_BUFFER, glm::value_ptr(colors.front()), 3, colors.size(), GL_STREAM_DRAW);
			m_scales.set_data(GL_ARRAY_BUFFER, scales.data(), 1, scales.size(), GL_STREAM_DRAW);

			// render
			renderer.set_program(m_shader);
			renderer.set_vertex_array(m_vertex_array);

			renderer.draw_indexed(GL_TRIANGLES, m_indices.get_element_count(), m_indices.get_component_type(), transforms.size());

			renderer.clear_vertex_array();
			renderer.clear_program();
		}

		
		asteroid_field::asteroid_field(entt::registry& registry, powerups& powerups, mbp_model const& model, std::vector<std::reference_wrapper<const mbp_model>> const& fragment_models, gl::shader_program const& depth_shader, gl::shader_program const& shader, gl::shader_program const& hit_shader):
			m_registry(registry),
			m_powerups(powerups),
			m_renderable(model, depth_shader, shader),
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
			m_hit_effects(registry, hit_shader),
			m_explosion_max_lifetime(high_res_duration_from_seconds(3.5f))
		{
			for (auto const& m : fragment_models)
				m_fragment_renderables.emplace_back(m, depth_shader, shader);
			
			m_fragment_renderable_instance_data.resize(m_fragment_renderables.size());

			for (auto const& m : fragment_models)
				m_fragment_renderable_transforms.push_back(m.get().m_transform);

			// set up collision effects
			{
				auto const color_map = std::map<float, glm::vec4>
				{
					{ 0.0f, { 0.2f, 0.2f, 0.2f, 1.f } },
					{ 0.7f, { 0.1f, 0.1f, 0.1f, 1.f } },
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
				m_hit_effects.set_blend_mode(gl::renderer::blending::BLEND);
				m_hit_effects.set_shadows_enabled(true);
			}

			spawn_wave();
		}

		asteroid_field::~asteroid_field()
		{
			auto view = m_registry.view<asteroid_data>();

			for (auto id : view)
				m_registry.destroy(id);

			for (auto const& e : m_asteroid_explosions)
				for (auto id : e.m_fragments)
					m_registry.destroy(id);
		}

		void asteroid_field::update(high_res_duration_t dt)
		{
			// update fragment lifetimes and remove expired fragments
			{
				auto view = m_registry.view<asteroid_fragment_data>();

				for (auto i = m_asteroid_explosions.begin(); i != m_asteroid_explosions.end(); )
				{
					auto last = std::remove_if(i->m_fragments.begin(), i->m_fragments.end(),
						[&] (entt::entity id)
						{
							auto& data = view.get<asteroid_fragment_data>(id);
							data.m_lifetime += dt;

							auto destroy = (data.m_lifetime >= data.m_max_lifetime);
							
							if (destroy)
								m_registry.destroy(id);
							
							return destroy;
						});

					i->m_fragments.erase(last, i->m_fragments.end());

					if (i->m_fragments.empty())
						i = m_asteroid_explosions.erase(i);
					else
						++i;
				}
			}

			auto to_destroy = std::vector<entt::entity>();
			
			struct destroyed_asteroid_data
			{
				asteroid_type m_type;
				glm::mat4 m_transform;
				glm::vec3 m_position;
				glm::vec3 m_velocity;
				glm::vec3 m_color;
				float m_scale;
			};

			auto destroyed_data = std::vector<destroyed_asteroid_data>();

			m_registry.view<asteroid_data, physics::rigidbody>().each(
				[&] (entt::entity e, asteroid_data& data, physics::rigidbody& rigidbody)
				{
					if (data.m_hp < 0)
					{
						to_destroy.push_back(e);
						destroyed_data.push_back({ data.m_type, rigidbody.get_transform(), rigidbody.get_position(), rigidbody.get_velocity(), data.m_color, data.m_model_scale });
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

				// add explosion
				{
					auto const fragment_count = m_fragment_renderables.size();

					auto fragment_ids = std::vector<entt::entity>();
					fragment_ids.reserve(fragment_count);

					for (auto i = std::size_t{ 0 }; i != fragment_count; ++i)
					{
						auto id = m_registry.create();

						auto& data = m_registry.emplace<asteroid_fragment_data>(id);
						data.m_model_index = i;
						data.m_lifetime = high_res_duration_t{ 0 };
						data.m_max_lifetime = high_res_duration_from_seconds(random::scale(m_rng, high_res_duration_to_seconds(m_explosion_max_lifetime), high_res_duration_to_seconds(m_explosion_max_lifetime) * 0.5f));

						auto const mass = random::scale(m_rng, 25.f, 5.f);
						auto const size = glm::vec3(random::scale(m_rng, 10.f, 5.f), random::scale(m_rng, 10.f, 5.f), random::scale(m_rng, 10.f, 5.f));

						auto transform = m_fragment_renderable_transforms[i];
						set_position(transform, get_position(transform) * destroyed.m_scale);
						transform = destroyed.m_transform * transform;
						auto position = get_position(transform);
						
						auto const vel_direction = glm::normalize(get_position(m_fragment_renderable_transforms[i]));
						auto const vel_magnitude = random::scale(m_rng, 15.f, 5.f);
						auto const vel_random = random::point_in_ring_3d(m_rng, 0.f, 5.f);
						auto const velocity = destroyed.m_velocity + vel_direction * vel_magnitude + vel_random;

						auto const ang_axis = random::point_in_ring_3d(m_rng, 0.f, 1.f);
						auto const ang_magnitude = random::scale(m_rng, 2.5f, 1.f);
						auto const ang_velocity = ang_axis * ang_magnitude;

						auto& rigidbody = m_registry.emplace<physics::rigidbody>(id);
						rigidbody.set_mass(mass);
						rigidbody.set_local_inertia_tensor(physics::make_cuboid_inertia_tensor(mass, size));
						rigidbody.set_position(position);
						rigidbody.set_velocity(velocity);
						rigidbody.set_angular_velocity(ang_velocity);

						fragment_ids.push_back(id);
					}

					m_asteroid_explosions.push_back({ destroyed.m_color, destroyed.m_scale, std::move(fragment_ids) });
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

		void asteroid_field::render_depth(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("asteroid_field::render_depth()");

			// render asteroids
			{
				auto view = m_registry.view<asteroid_data, physics::rigidbody>();

				if (!view.empty())
				{
					for (auto id : view)
					{
						auto [a, rb] = view.get<asteroid_data, physics::rigidbody>(id);
						auto const transform = rb.get_transform();
						m_renderable_instance_data.m_transforms.push_back(matrices.model_view_projection_matrix(transform));
						m_renderable_instance_data.m_scales.push_back(a.m_model_scale);
					}

					m_renderable.render_depth(renderer, matrices, m_renderable_instance_data.m_transforms, m_renderable_instance_data.m_scales);

					m_renderable_instance_data.clear();
				}
			}

			// render fragments
			{
				auto view = m_registry.view<asteroid_fragment_data, physics::rigidbody>();

				if (!view.empty())
				{
					for (auto const& e : m_asteroid_explosions)
					{
						for (auto id : e.m_fragments)
						{
							auto [f, rb] = view.get<asteroid_fragment_data, physics::rigidbody>(id);
							auto const transform = rb.get_transform();

							auto& data = m_fragment_renderable_instance_data[f.m_model_index];
							data.m_transforms.push_back(matrices.model_view_projection_matrix(transform));
							data.m_scales.push_back(e.m_model_scale);
						}
					}

					for (auto i = std::size_t{ 0 }; i != m_fragment_renderables.size(); ++i)
					{
						auto const& data = m_fragment_renderable_instance_data[i];
						m_fragment_renderables[i].render_depth(renderer, matrices, data.m_transforms, data.m_scales);
					}

					for (auto& i : m_fragment_renderable_instance_data)
						i.clear();
				}
			}
		}
		
		void asteroid_field::render_scene(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("asteroid_field::render_scene()");

			// render asteroids
			{
				auto view = m_registry.view<asteroid_data, physics::rigidbody>();

				if (!view.empty())
				{
					for (auto id : view)
					{
						auto [a, rb] = view.get<asteroid_data, physics::rigidbody>(id);
						auto const transform = rb.get_transform();
						m_renderable_instance_data.m_transforms.push_back(matrices.model_view_projection_matrix(transform));
						m_renderable_instance_data.m_normal_matrices.push_back(matrices.normal_matrix(transform));
						m_renderable_instance_data.m_colors.push_back(a.m_color);
						m_renderable_instance_data.m_scales.push_back(a.m_model_scale);
					}

					m_renderable.render_scene(renderer, matrices, m_renderable_instance_data.m_transforms, m_renderable_instance_data.m_normal_matrices, m_renderable_instance_data.m_colors, m_renderable_instance_data.m_scales);

					m_renderable_instance_data.clear();
				}
			}

			// render fragments
			{
				auto view = m_registry.view<asteroid_fragment_data, physics::rigidbody>();

				if (!view.empty())
				{
					for (auto const& e : m_asteroid_explosions)
					{
						for (auto id : e.m_fragments)
						{
							auto [f, rb] = view.get<asteroid_fragment_data, physics::rigidbody>(id);
							auto const transform = rb.get_transform();

							auto& data = m_fragment_renderable_instance_data[f.m_model_index];
							data.m_transforms.push_back(matrices.model_view_projection_matrix(transform));
							data.m_normal_matrices.push_back(matrices.normal_matrix(transform));
							data.m_colors.push_back(e.m_color);
							data.m_scales.push_back(e.m_model_scale);
						}
					}

					for (auto i = std::size_t{ 0 }; i != m_fragment_renderables.size(); ++i)
					{
						auto const& data = m_fragment_renderable_instance_data[i];
						m_fragment_renderables[i].render_scene(renderer, matrices, data.m_transforms, data.m_normal_matrices, data.m_colors, data.m_scales);
					}

					for (auto& i : m_fragment_renderable_instance_data)
						i.clear();
				}
			}
		}
		
		void asteroid_field::render_particles(gl::renderer& renderer, camera_matrices const& light_matrices, camera_matrices const& matrices, gl::texture_2d const& shadow_map)
		{
			ZoneScopedN("asteroid_field::render_particles()");

			m_hit_effects.render(renderer, light_matrices, matrices, shadow_map);
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
			rigidbody.set_angular_factor({ 0.f, 0.f, 0.f }); // no rotation!
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