#include "bump_game_powerups.hpp"

#include "bump_camera.hpp"
#include "bump_game_player.hpp"
#include "bump_log.hpp"
#include "bump_physics.hpp"
#include "bump_mbp_model.hpp"

namespace bump
{
	
	namespace game
	{
		
		powerups::powerups(entt::registry& registry,
				gl::shader_program const& shader,
				mbp_model const& shield_model,
				mbp_model const& armor_model,
				mbp_model const& lasers_model):
			m_registry(registry),
			m_shield_renderable(shader, shield_model),
			m_armor_renderable(shader, armor_model),
			m_lasers_renderable(shader, lasers_model),
			m_max_lifetime(high_res_duration_from_seconds(5.f))
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

			m_entities.push_back(id);
		}

		void powerups::update(high_res_duration_t dt)
		{
			auto view = m_registry.view<powerup_data>();

			for (auto id : view)
				view.get<powerup_data>(id).m_lifetime += dt;
			
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

		void powerups::render(gl::renderer& renderer, camera_matrices const& matrices)
		{
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