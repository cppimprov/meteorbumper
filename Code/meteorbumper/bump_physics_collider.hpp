#pragma once

#include <entt.hpp>
#include <glm/glm.hpp>

#include <cstdint>
#include <functional>
#include <optional>
#include <variant>

namespace bump
{
	
	namespace physics
	{

		class rigidbody;
	
		struct sphere_shape
		{
			float m_radius = 1.f;
		};

		struct inverse_sphere_shape
		{
			float m_radius = 1.f;
		};

		enum collision_layers : std::uint32_t
		{
			PLAYER =         1u << 0u,
			PLAYER_WEAPONS = 1u << 1u,
			ASTEROIDS =      1u << 2u,
			BOUNDS =         1u << 3u,
			POWERUPS =       1u << 4u,
			PARTICLES =      1u << 5u,
		};
		
		struct collision_data
		{
			glm::vec3 m_point;
			glm::vec3 m_normal;
			float m_penetration = 0.f;
		};

		class collider
		{
		public:

			using shape_type = std::variant<sphere_shape, inverse_sphere_shape>;
			using callback_type = std::function<void(entt::entity, collision_data const&, float)>;

			explicit collider();

			void set_shape(shape_type shape) { m_shape = shape; }
			shape_type get_shape() const { return m_shape; }

			void set_restitution(float value) { m_restitution = glm::clamp(value, 0.f, 1.f); }
			float get_restitution() const { return m_restitution; }

			void set_collision_layer(std::uint32_t layer) { m_layer = layer; }
			std::uint32_t get_collision_layer() const { return m_layer; }

			void set_collision_mask(std::uint32_t layer_mask) { m_layer_mask = layer_mask; }
			std::uint32_t get_collision_mask() const { return m_layer_mask; }

			void set_callback(callback_type callback) { m_callback = std::move(callback); }
			void call_callback(entt::entity other, collision_data const& data, float relative_velocity_magnitude) const { if (m_callback) m_callback(other, data, relative_velocity_magnitude); }

		private:

			float m_restitution;
			shape_type m_shape;
			std::uint32_t m_layer;      // bitmask of layers this object is on
			std::uint32_t m_layer_mask; // bitmask of layers this object collides with
			callback_type m_callback;   // note: callback must not do anything to invalidate this collider (e.g. spawn entities with colliders).
		};

		std::optional<collision_data> dispatch_find_collision(rigidbody const& p1, collider const& c1, rigidbody const& p2, collider const& c2);
		void resolve_impulse(rigidbody& a, rigidbody& b, collision_data const& c, float e); // e == restitution
		void resolve_projection(rigidbody& a, rigidbody& b, collision_data const& c);
		
	} // physics
	
} // bumpe