#include "bump_mbp_model.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"
#include "bump_json_glm.hpp"

#include <json.hpp>

#include <fstream>

namespace bump
{
	
	namespace
	{

		mbp_material parse_mbp_material_json(nlohmann::json j)
		{
			if (!j.is_object())
				throw std::runtime_error("Material must be an object.");
			
			auto out = mbp_material();
			out.m_name = j.at("name").get<std::string>();
			out.m_base_color = j.at("base_color").get<glm::vec3>();
			out.m_emissive_color = j.at("emissive_color").get<glm::vec3>();
			out.m_metallic = j.at("metallic").get<float>();
			out.m_specular = j.at("specular").get<float>();
			out.m_roughness = j.at("roughness").get<float>();
			out.m_alpha = j.at("alpha").get<float>();
			out.m_ior = j.at("ior").get<float>();
			return out;
		}

		mbp_mesh parse_mbp_mesh_json(nlohmann::json j)
		{
			if (!j.is_object())
				throw std::runtime_error("Mesh must be an object.");
			
			auto out = mbp_mesh();
			out.m_vertices = j.at("vertices").get<std::vector<float>>();
			out.m_texture_coords = j.at("texture_coords").get<std::vector<std::vector<float>>>();
			out.m_normals = j.at("normals").get<std::vector<float>>();
			out.m_tangents = j.at("tangents").get<std::vector<float>>();
			out.m_bitangents = j.at("bitangents").get<std::vector<float>>();
			out.m_indices = j.at("indices").get<std::vector<std::uint32_t>>();
			return out;
		}

		mbp_model parse_mbp_model_json(nlohmann::json j)
		{
			if (!j.is_object())
				throw std::runtime_error("Root element must be an object.");
			
			auto transform = j.at("transform").get<glm::mat4>();

			auto submeshes = std::vector<mbp_submesh>();
			for (auto j_submesh : j.at("submeshes"))
			{
				auto material = parse_mbp_material_json(j_submesh.at("material"));
				auto mesh = parse_mbp_mesh_json(j_submesh.at("mesh"));
				auto submesh = mbp_submesh{ std::move(material), std::move(mesh) };

				submeshes.push_back(std::move(submesh));
			}

			return { transform, std::move(submeshes) };
		}

	} // unnamed

	mbp_model load_mbp_model_json(std::string const& filename)
	{
		try
		{
			auto file = std::ifstream(filename, std::ios_base::binary); // todo: widen filename for windows!

			if (!file.is_open())
			{
				log_error("Failed to open mbp_model file: " + filename);
				die();
			}

			auto j = nlohmann::json::parse(file);

			return parse_mbp_model_json(j);
		}
		catch (nlohmann::json::parse_error const& e)
		{
			log_error("Failed to parse json in mbp_model file: " + filename);
			log_error("Error id: " + std::to_string(e.id));
			log_error("Error message: \n" + std::string(e.what()));
			die();
		}
		catch (nlohmann::json::exception const& e)
		{
			log_error("Failed to parse mbp_model file: " + filename);
			log_error("Error id: " + std::to_string(e.id));
			log_error("Error message:\n" + std::string(e.what()));
			die();
		}
		catch (std::runtime_error const& e)
		{
			log_error("Failed to parse mesh data in mbp_model file: " + filename);
			log_error("Error message:\n" + std::string(e.what()));
			die();
		}
		
		die();
		return {};
	}
	
} // bump