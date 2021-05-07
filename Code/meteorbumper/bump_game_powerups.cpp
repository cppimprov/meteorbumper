#include "bump_game_powerups.hpp"

#include "bump_camera.hpp"
#include "bump_game_player.hpp"
#include "bump_lighting.hpp"
#include "bump_log.hpp"
#include "bump_physics.hpp"
#include "bump_mbp_model.hpp"

#include <Tracy.hpp>

namespace bump
{
	
	namespace game
	{

		namespace
		{

			auto const shield_color = glm::vec3{ 0.08022f, 0.637597f, 0.672443f };
			auto const armor_color = glm::vec3{ 0.83077f, 0.799103f, 0.043735f };
			auto const laser_color = glm::vec3{ 0.05448f, 0.814847f, 0.090842f };

			auto const position_offset = glm::vec3{ 0.f, -01.5f, 0.f };

		} // unnamed
		
		powerups::powerups(entt::registry& registry,
				gl::shader_program const& depth_shader,
				gl::shader_program const& shader,
				mbp_model const& shield_model,
				mbp_model const& armor_model,
				mbp_model const& lasers_model):
			m_registry(registry),
			m_shield_renderable(depth_shader, shader, shield_model),
			m_armor_renderable(depth_shader, shader, armor_model),
			m_lasers_renderable(depth_shader, shader, lasers_model),
			m_max_lifetime(high_res_duration_from_seconds(5.f)),
			m_light_colors{ 
				{ powerup_type::RESET_SHIELDS, shield_color }, 
				{ powerup_type::RESET_ARMOR, armor_color }, 
				{ powerup_type::UPGRADE_LASERS, laser_color } }
		{
			
		}

		powerups::~powerups()
		{
			for (auto id : m_entities)
				m_registry.destroy(id);
		}
		
		void powerups::spawn(glm::vec3 position, powerup_type type)
		{
			auto id = m_registry.create();

			auto& data = m_registry.emplace<powerup_data>(id);
			data.m_type = type;
			data.m_lifetime = high_res_duration_t{ 0 };
			data.m_collected = false;

			auto const mass_kg = 5.f;
			auto const radius_m = 1.25f;

			auto& rigidbody = m_registry.emplace<physics::rigidbody>(id);
			rigidbody.set_mass(mass_kg);
			rigidbody.set_local_inertia_tensor(physics::make_sphere_inertia_tensor(mass_kg, radius_m));
			rigidbody.set_position(position);

			auto& collider = m_registry.emplace<physics::collider>(id);
			collider.set_shape({ physics::sphere_shape{ radius_m } });
			collider.set_collision_layer(physics::collision_layers::POWERUPS);
			collider.set_collision_mask(physics::collision_layers::PLAYER);

			auto callback = [=] (entt::entity other, physics::collision_data const&, float)
			{
				if (m_registry.has<player_tag>(other))
					m_registry.get<powerup_data>(id).m_collected = true;
			};

			collider.set_callback(std::move(callback));

			auto& light = m_registry.emplace<lighting::point_light>(id);
			light.m_color = m_light_colors.at(type) * 500.f;
			light.m_position = position + position_offset;
			light.m_radius = 35.f;

			m_entities.push_back(id);
		}

		void powerups::update(high_res_duration_t dt)
		{
			auto view = m_registry.view<powerup_data, physics::rigidbody, lighting::point_light>();

			for (auto id : view)
			{
				view.get<powerup_data>(id).m_lifetime += dt;
				view.get<lighting::point_light>(id).m_position = view.get<physics::rigidbody>(id).get_position() + position_offset;
			}
			
			auto first_dead_entity = std::remove_if(m_entities.begin(), m_entities.end(),
				[&] (entt::entity id)
				{
					auto const& data = view.get<powerup_data>(id);
					auto result = data.m_collected || data.m_lifetime > m_max_lifetime;

					if (result)
						m_registry.destroy(id);
					
					return result;
				});

			m_entities.erase(first_dead_entity, m_entities.end());
		}

		void powerups::render_depth(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("powerups::render_depth()");

			auto view = m_registry.view<powerup_data, physics::rigidbody>();

			for (auto id : view)
			{
				auto [data, rb] = view.get<powerup_data, physics::rigidbody>(id);

				if (data.m_type == powerup_type::RESET_SHIELDS)
				{
					m_shield_renderable.set_transform(rb.get_transform());
					m_shield_renderable.render_depth(renderer, matrices);
				}
				else if (data.m_type == powerup_type::RESET_ARMOR)
				{
					m_armor_renderable.set_transform(rb.get_transform());
					m_armor_renderable.render_depth(renderer, matrices);
				}
				else if (data.m_type == powerup_type::UPGRADE_LASERS)
				{
					m_lasers_renderable.set_transform(rb.get_transform());
					m_lasers_renderable.render_depth(renderer, matrices);
				}
			}
		}

		void powerups::render_scene(gl::renderer& renderer, camera_matrices const& matrices)
		{
			ZoneScopedN("powerups::render_scene()");

			auto view = m_registry.view<powerup_data, physics::rigidbody>();

			for (auto id : view)
			{
				auto [data, rb] = view.get<powerup_data, physics::rigidbody>(id);

				if (data.m_type == powerup_type::RESET_SHIELDS)
				{
					m_shield_renderable.set_transform(rb.get_transform());
					m_shield_renderable.render(renderer, matrices);
				}
				else if (data.m_type == powerup_type::RESET_ARMOR)
				{
					m_armor_renderable.set_transform(rb.get_transform());
					m_armor_renderable.render(renderer, matrices);
				}
				else if (data.m_type == powerup_type::UPGRADE_LASERS)
				{
					m_lasers_renderable.set_transform(rb.get_transform());
					m_lasers_renderable.render(renderer, matrices);
				}
			}
		}
		
	} // game
	
} // bump