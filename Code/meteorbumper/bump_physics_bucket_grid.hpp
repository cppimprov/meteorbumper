#pragma once

#include "bump_physics_rigidbody.hpp"
#include "bump_physics_collider.hpp"
#include "bump_narrow_cast.hpp"

#include <entt.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/std_based_type.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/integer.hpp>

#include <Tracy.hpp>

#include <vector>

namespace bump
{
	
	namespace physics
	{
		
		class bucket_grid
		{
		public:

			explicit bucket_grid(glm::vec3 cell_size, glm::size3 bucket_count):
				m_cell_size(cell_size),
				m_bucket_count(bucket_count),
				m_grid_size(m_cell_size * glm::vec3(m_bucket_count))
			{

			}

			template<class ViewT>
			void create(ViewT const& view)
			{
				ZoneScopedN("bucket_grid::create()");

				clear();

				m_buckets.resize(glm::compMul(m_bucket_count));

				for (auto id : view)
				{
					auto [rb, c] = view.get<rigidbody, collider>(id);

					auto aabb = dispatch_get_aabb(rb, c);
					die_if(!aabb.is_valid());

					auto min_cell = glm::ivec3(aabb.min / m_cell_size);
					auto max_cell = glm::ivec3(aabb.max / m_cell_size) + 1;

					for (auto z = min_cell.z; z != max_cell.z; ++z)
					{
						for (auto y = min_cell.y; y != max_cell.y; ++y)
						{
							for (auto x = min_cell.x; x != max_cell.x; ++x)
							{
								auto const i = mod(glm::ivec3{ x, y, z }, glm::ivec3(m_bucket_count));
								auto const bucket_index = (std::size_t)(i.x + i.y * m_bucket_count.x + i.z * m_bucket_count.x * m_bucket_count.y);
								die_if(bucket_index >= m_buckets.size());

								m_buckets[bucket_index].push_back(id);
							}
						}
					}
				}
			}

			template<class ViewT, class OutIt>
			void get_collision_pairs(ViewT view, OutIt output) const
			{
				ZoneScopedN("bucket_grid::get_collision_pairs()");

				die_if(m_buckets.empty());

				for (auto id : view)
				{
					auto [rb, c] = view.get<rigidbody, collider>(id);

					auto aabb = dispatch_get_aabb(rb, c);
					die_if(!aabb.is_valid());

					auto min_cell = glm::ivec3(aabb.min / m_cell_size);
					auto max_cell = glm::ivec3(aabb.max / m_cell_size) + 1;

					for (auto z = min_cell.z; z != max_cell.z; ++z)
					{
						for (auto y = min_cell.y; y != max_cell.y; ++y)
						{
							for (auto x = min_cell.x; x != max_cell.x; ++x)
							{
								auto const i = mod(glm::ivec3{ x, y, z }, glm::ivec3(m_bucket_count));
								auto const bucket_index = std::size_t(i.x + i.y * m_bucket_count.x + i.z * m_bucket_count.x * m_bucket_count.y);
								die_if(bucket_index >= m_buckets.size());

								for (auto e : m_buckets[bucket_index])
									if (e != id) // prevent self-collision
										*output++ = { id, e };
							}
						}
					}
				}
			}

			void clear()
			{
				m_buckets.clear();
			}

		private:

			static glm::ivec3 mod(glm::ivec3 a, glm::ivec3 b)
			{
				return { glm::mod(a.x, b.x), glm::mod(a.y, b.y), glm::mod(a.z, b.z) };
			}

			glm::vec3 m_cell_size;
			glm::size3 m_bucket_count;
			glm::vec3 m_grid_size; // m_cell_size * m_bucket_count
			std::vector<std::vector<entt::entity>> m_buckets; // todo: would a map be better? (fewer empty vectors sitting around)
		};
		
	} // physics
	
} // bump