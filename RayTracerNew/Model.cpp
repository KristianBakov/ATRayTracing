#include "Model.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

struct Vertex
{
	vec3 Position;
	vec3 Normal;
	vec3 TexCoord;
	vec3 Color;
};

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
}
