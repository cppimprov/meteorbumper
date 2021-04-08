#include "bump_game_power_ups.hpp"

#include "bump_camera.hpp"
#include "bump_log.hpp"
#include "bump_physics.hpp"

namespace bump
{
	
	namespace game
	{
		
		power_ups::power_ups(entt::registry& registry):
			m_registry(registry),
			m_max_lifetime(std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(5.f)))
		{
			// ...
		}
		
		void power_ups::spawn(glm::vec3 position, power_up_type type)
		{
			auto id = m_registry.create();

			auto& data = m_registry.emplace<power_up_data>(id);
			data.m_type = type;
			data.m_lifetime = high_res_duration_t{ 0 };
			data.m_collected = false;

			auto const mass_kg = 5.f;
			auto const radius_m = 5.f;

			auto& rigidbody = m_registry.emplace<physics::rigidbody>(id);
			rigidbody.set_mass(mass_kg);
			rigidbody.set_local_inertia_tensor(physics::make_sphere_inertia_tensor(mass_kg, radius_m));
			rigidbody.set_position(position);

			auto& collider = m_registry.emplace<physics::collider>(id);
			collider.set_shape({ physics::sphere_shape{ radius_m } });
			collider.set_collision_layer(physics::collision_layers::POWERUPS);
			collider.set_collision_mask(physics::collision_layers::PLAYER);

			auto callback = [=] (entt::entity, physics::collision_data const&, float)
			{
				// note: should only collide w/ player, so don't check the entity type
				m_registry.get<power_up_data>(id).m_collected = true;
			};

			collider.set_callback(std::move(callback));

			m_entities.push_back(id);
		}

		void power_ups::update(high_res_duration_t dt)
		{
			auto view = m_registry.view<power_up_data>();

			for (auto& id : view)
				view.get<power_up_data>(id).m_lifetime += dt;
			
			auto first_dead_entity = std::remove_if(m_entities.begin(), m_entities.end(),
				[&] (entt::entity id)
				{
					auto const& data = view.get<power_up_data>(id);
					auto result = data.m_collected || data.m_lifetime > m_max_lifetime;

					if (result)
						m_registry.destroy(id);
					
					return result;
				});

			m_entities.erase(first_dead_entity, m_entities.end());
		}

		// void power_ups::render(gl::renderer& renderer, camera_matrices const& matrices)
		// {
		// 	// ...
		// }
		
	} // game
	
} // bump