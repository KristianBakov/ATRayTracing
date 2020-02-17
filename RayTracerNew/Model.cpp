#include "Model.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

bool Model::LoadModel(std::string _modelPath, std::string file, std::unique_ptr<material> mat)
{

	m_material = std::move(mat);
	
	std::string inputfile = "cornell_box.obj";
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str(), file.c_str(), true);
	if (!err.empty() || !warn.empty())
	{
		std::cout << warn + err + "\n";
		return false;
	}
	if (!ret)
	{
		exit(1);
	}

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex;

			vertex.Position = vec3(
				attrib.vertices[3 * index.vertex_index],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]);

			if (index.texcoord_index >= 0)
			{
				vertex.TexCoord = vec3(
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1], 0.0f);
			}
			else
			{
				vertex.TexCoord = vec3(0, 0, 0);
			}
			
			m_model.push_back(vertex);
			
		}
	}

	if (m_model.size() % 3 != 0)
	{
		std::cout << "One of the triangles is not a triangle" << std::endl;
		exit(1);
	}

	vec3 min = m_model.at(0).Position;
	vec3 max = m_model.at(0).Position;
	for (const auto& vertex : m_model)
	{
		min.e[0] = min.e[0] < vertex.Position.e[0] ? min.e[0] : vertex.Position.e[0];
		min.e[1] = min.e[1] < vertex.Position.e[1] ? min.e[1] : vertex.Position.e[1];
		min.e[2] = min.e[2] < vertex.Position.e[2] ? min.e[2] : vertex.Position.e[2];

		max.e[0] = max.e[0] < vertex.Position.e[0] ? max.e[0] : vertex.Position.e[0];
		max.e[1] = max.e[1] < vertex.Position.e[1] ? max.e[1] : vertex.Position.e[1];
		max.e[2] = max.e[2] < vertex.Position.e[2] ? max.e[2] : vertex.Position.e[2];
	}

	m_bounding_box = aabb(min, max);
}

bool Model::hit(const ray& ray, float t_min, float t_max, hit_record& record) const
{
	for (size_t i = 0; i < m_model.size(); i += 3)
	{
		//bool result = 
	}
}

bool Model::rayTriangleIntersect(const ray& ray, float t_min, float t_max, hit_record& record, const Vertex& v0, const Vertex& v1, const Vertex& v2) const
{
	vec3 v0v1 = v1.Position - v0.Position;
	vec3 v0v2 = v2.Position - v0.Position;

	vec3 pvec = ray.direction().cross(ray.direction(), v0v2);
	//float det = v0v1.

}
