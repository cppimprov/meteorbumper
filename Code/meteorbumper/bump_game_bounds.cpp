#include "bump_game_bounds.hpp"

#include "bump_physics.hpp"

namespace bump
{
	
	namespace game
	{
		
		bounds::bounds(entt::registry& registry, float radius):
			m_registry(registry),
			m_id(entt::null)
		{
			m_id = registry.create();
			
			registry.emplace<bounds_tag>(m_id);

			auto& rigidbody = registry.emplace<physics::rigidbody>(m_id);
			rigidbody.set_infinite_mass();

			auto& collider = registry.emplace<physics::collider>(m_id);
			collider.set_shape(physics::inverse_sphere_shape{ radius });
			collider.set_collision_layer(physics::BOUNDS);
			collider.set_collision_mask(~physics::PLAYER_WEAPONS);
		}

		bounds::~bounds()
		{
			m_registry.destroy(m_id);
			m_id = entt::null;
		}

	} // game
	
} // bump