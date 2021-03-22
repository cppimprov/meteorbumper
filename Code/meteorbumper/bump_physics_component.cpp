#include "bump_physics_component.hpp"

#include "bump_physics_inertia_tensor.hpp"

namespace bump
{
	
	namespace physics
	{
		
		physics_component::physics_component():
			// mass
			m_inverse_mass(1.f),
			m_local_inertia_tensor(make_sphere_inertia_tensor(1.f, 1.f)),
			// movement
			m_position(0.f),
			m_orientation(),
			m_velocity(0.f),
			m_angular_velocity(0.f),
			m_linear_damping(0.99999f),
			m_angular_damping(0.99999f),
			// forces
			m_force(0.f),
			m_torque(0.f)
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
				m_velocity *= m_linear_damping;
			}

			// integrate for orientation
			{
				auto acceleration = get_inverse_inertia_tensor() * m_torque;

				m_angular_velocity += acceleration * dt_s;
				m_orientation += m_orientation * glm::quat(0.f, m_angular_velocity) * (dt_s / 2.f);
				m_orientation = glm::normalize(m_orientation);
				m_angular_velocity *= m_angular_damping;
			}
		}
		
	} // physics
	
} // bump