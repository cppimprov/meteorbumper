#pragma once

#include "bump_die.hpp"
#include "bump_time.hpp"
#include "bump_transform.hpp"

#include <entt.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace bump
{

	namespace game::ecs
	{

		inline glm::mat3 make_sphere_inertia_tensor(float mass, float radius)
		{
			auto value = (2.f * mass * (radius * radius)) / 5.f;
			return glm::mat3(value);
		}

		// todo: other inertia tensors!
		// todo: combine inertia tensors!

		class physics_component;
	
		class physics_system
		{
		public:

			physics_system(high_res_duration_t update_time = 
				std::chrono::duration_cast<high_res_duration_t>(std::chrono::duration<float>(1.f / 120.f)));

			void update(entt::registry& registry, high_res_duration_t dt);

		private:

			high_res_duration_t m_update_time;
			high_res_duration_t m_accumulator;

			// todo: collision pairs / data
			// todo: force generators
		};

		class physics_component
		{
		public:

			physics_component();

			// mass
			bool has_mass() const { return m_inverse_mass != 0.f; }
			bool has_infinite_mass() const { return !has_mass(); }

			float get_mass() const { die_if(!has_mass());  return 1.f / m_inverse_mass; }
			float get_inverse_mass() const { return m_inverse_mass; }

			void set_mass(float mass) { m_inverse_mass = 1.f / glm::clamp(mass, 0.01f, 10000.f); }
			void set_infinite_mass() { m_inverse_mass = 0.f; }

			void set_local_inertia_tensor(glm::mat3 inertia_tensor) { m_local_inertia_tensor = inertia_tensor; }
			glm::mat3 get_local_inertia_tensor() const { return has_mass() ? m_local_inertia_tensor : glm::mat3(0.f); }
			glm::mat3 get_inverse_inertia_tensor() const { return has_mass() ? glm::inverse(glm::mat3_cast(m_orientation) * m_local_inertia_tensor) : glm::mat3(0.f); }

			// movement
			void set_linear_damping(float multiplier) { m_linear_damping = glm::clamp(multiplier, 0.f, 1.f); }
			float get_linear_damping() const { return m_linear_damping; }

			void set_angular_damping(float multiplier) { m_angular_damping = glm::clamp(multiplier, 0.f, 1.f); }
			float get_angular_damping() const { return m_angular_damping; }

			void set_position(glm::vec3 position) { m_position = position; }
			glm::vec3 get_position() const { return m_position; }

			void set_velocity(glm::vec3 velocity) { m_velocity = velocity; }
			glm::vec3 get_velocity() const { return m_velocity; }

			void set_orientation(glm::quat orientation) { m_orientation = orientation; }
			glm::quat get_orientation() const { return m_orientation; }

			void set_angular_velocity(glm::vec3 angular_velocity) { m_angular_velocity = angular_velocity; }
			glm::vec3 get_angular_velocity() const { return m_angular_velocity; }
			
			void set_transform(glm::mat4 transform) { m_position = bump::get_position(transform); m_orientation = bump::get_rotation(transform); }
			glm::mat4 get_transform() const { auto t = glm::translate(glm::mat4(1.f), m_position); t *= glm::mat4_cast(m_orientation); return t; }

			// forces
			void add_force(glm::vec3 force) { m_force += force; }
			void clear_force() { m_force = glm::vec3(0.f); }

			void add_torque(glm::vec3 torque) { m_torque += torque; }
			void clear_torque() { m_torque = glm::vec3(0.f); }

			// todo: is this correct? should point be in object coordinates?
			void add_force_at_point(glm::vec3 force, glm::vec3 point) { add_force(force); add_torque(glm::cross(point - m_position, force)); }

			glm::vec3 get_force() const { return m_force; }
			glm::vec3 get_torque() const { return m_torque; }

			// shape
			// ...

			// material
			float set_restitution(float value) { m_restitution = glm::clamp(value, 0.f, 1.f); }
			float get_restitution() const { return m_restitution; }
			
		private:

			void update(high_res_duration_t dt);
			friend class physics_system;

			// mass
			float m_inverse_mass;
			glm::mat3 m_local_inertia_tensor;

			// movement
			glm::vec3 m_position;
			glm::quat m_orientation;
			glm::vec3 m_velocity;
			glm::vec3 m_angular_velocity;
			float m_linear_damping;
			float m_angular_damping;
		
			// forces
			glm::vec3 m_force;
			glm::vec3 m_torque;

			// shape
			// ...

			// material
			float m_restitution;
		};

	} // game::ecs
	
} // bump