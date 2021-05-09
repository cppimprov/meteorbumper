#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace bump
{

	struct mbp_material
	{
		std::string m_name;
		glm::vec3 m_base_color;
		glm::vec3 m_emissive_color;
		float m_metallic;
		float m_specular;
		float m_roughness;
		float m_alpha;
		float m_ior;
	};

	struct mbp_mesh
	{
		std::vector<float> m_vertices; // 3 per vertex
		std::vector<std::vector<float>> m_texture_coords; // 2 per vertex for each layer (may be empty)
		std::vector<float> m_normals; // 3 per vertex (or empty)
		std::vector<float> m_tangents; // 3 per vertex (or empty)
		std::vector<float> m_bitangents; // 3 per vertex (or empty)
		std::vector<std::uint32_t> m_indices; // 3 per face
	};

	struct mbp_submesh
	{
		mbp_material m_material;
		mbp_mesh m_mesh;
	};
	
	struct mbp_model
	{
		glm::mat4 m_transform;
		std::vector<mbp_submesh> m_submeshes;
	};

	mbp_model load_mbp_model_json(std::string const& filename);
	
} // bump