#include "bump_game_ecs_physics.hpp"

#include "bump_die.hpp"

namespace bump
{	

	namespace game::ecs
	{

		physics_system::physics_system(high_res_duration_t update_time):
			m_update_time(update_time),
			m_accumulator(0) { }

		void physics_system::update(entt::registry& registry, high_res_duration_t dt)
		{
			m_accumulator += dt;

			// note: if we start destroying physics objects inside the physics update loop (e.g. on collision)
			// we will need to update this view inside the accumulator!
			auto view = registry.view<physics_component>();

			while (m_accumulator >= m_update_time)
			{
				// collisions... (broad phase, narrow_phase, resolve, notify)

				for (auto id : view)
					view.get<physics_component>(id).update(m_update_time);

				m_accumulator -= m_update_time;
			}

			for (auto id : view)
			{
				auto& c = view.get<physics_component>(id);
				c.clear_force();
				c.clear_torque();
			}
		}
		
		physics_component::physics_component():
			// mass
			m_inverse_mass(1.f),
			m_local_inertia_tensor(make_sphere_inertia_tensor(1.f, 1.f)),
			// movement
			m_position(0.f),
			m_orientation(),
			m_velocity(0.f),
			m_angular_velocity(0.f),
			m_damping(0.99999f),
			// forces
			m_force(0.f),
			m_torque(0.f),
			// shape
			// ...
			// material
			m_restitution(0.5f)
			{ }
		
		void physics_component::update(high_res_duration_t dt)
		{
			auto dt_s = std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();

			// integrate for position
			{
				auto acceleration = m_inverse_mass * m_force;

				// semi-implicit euler
				m_velocity += acceleration * dt_s;
				m_position += m_velocity * dt_s;
				m_velocity *= m_damping;
			}

			// integrate for orientation
			{
				auto acceleration = get_inverse_inertia_tensor() * m_torque;

				m_angular_velocity += acceleration * dt_s;
				m_orientation += m_orientation * glm::quat(0.f, m_angular_velocity) * (dt_s / 2.f);
				m_orientation = glm::normalize(m_orientation);
				m_angular_velocity *= m_damping;
			}
		}
		
	} // game::ecs
	
} // bump
