#pragma once
#ifndef MODELH
#define MODELH

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "aabb.h"

struct Vertex
{
	vec3 Position;
	vec3 TexCoord;
};

class Model : public hittable
{
public:
	Model() = default;
	Model(std::string _modelPath, std::string file, material* mat);
	virtual bool hit(const ray& ray, float t_min, float t_max, hit_record& record)const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const;

	//virtual bool bounding_box(float t0, float t1, aabb& box) const;
	bool rayTriangleIntersect(const ray& ray, float t_min, float t_max, hit_record& record, const Vertex& v0, const Vertex& v1, const Vertex& v2)const;

	std::vector<Vertex> m_model;
	material* m_material;
	
};

Model::Model(std::string _modelPath, std::string file, material* mat)
{

	m_material = mat;

	
	//std::string inputfile = "cornell_box.obj";
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	std::string filepath = _modelPath + file;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str(), _modelPath.c_str(), true);
	if (!err.empty() || !warn.empty())
	{
		std::cout << warn + err + "\n";
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
				attrib.vertices[3 * index.vertex_index + 0],
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

}

bool Model::hit(const ray& ray, float t_min, float t_max, hit_record& record) const
{
	for (size_t i = 0; i < m_model.size(); i += 3)
	{
		bool result = rayTriangleIntersect(ray, t_min, t_max, record, m_model.at(i), m_model.at(i + 1), m_model.at(i + 2));
		if (result) { return true; }
	}
}

bool Model::bounding_box(float t0, float t1, aabb& box) const
{
	vec3 minVertex = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	vec3 maxVertex = vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	//Loop through the vertices describing the bounding box
	for (const auto& Vert : m_model)
	{
		//Get the smallest vertex 
		minVertex.e[0] = minVertex.e[0] < Vert.Position.e[0] ? minVertex.e[0] : Vert.Position.e[0];    // Find smallest x value in model
		minVertex.e[1] = minVertex.e[1] < Vert.Position.e[1] ? minVertex.e[1] : Vert.Position.e[1];     // Find smallest y value in model
		minVertex.e[2] = minVertex.e[2] < Vert.Position.e[2] ? minVertex.e[2] : Vert.Position.e[2];    // Find smallest z value in model

		//Get the largest vertex 
		maxVertex.e[0] = minVertex.e[0] > Vert.Position.e[0] ? minVertex.e[0] : Vert.Position.e[0];    // Find largest x value in model
		maxVertex.e[1] = minVertex.e[1] > Vert.Position.e[1] ? minVertex.e[1] : Vert.Position.e[1];    // Find largest y value in model
		maxVertex.e[2] = minVertex.e[2] > Vert.Position.e[2] ? minVertex.e[2] : Vert.Position.e[2];    // Find largest z value in model
	}
	//create box
	box = aabb(minVertex, maxVertex);
	return true;
}

bool Model::rayTriangleIntersect(const ray& ray, float t_min, float t_max, hit_record& record, const Vertex& v0, const Vertex& v1, const Vertex& v2) const
{
	vec3 v0v1 = v1.Position - v0.Position;
	vec3 v0v2 = v2.Position - v0.Position;

	vec3 pvec = cross(ray.direction(), v0v2);
	float det = dot(v0v1, pvec);
	if (det < FLT_EPSILON) return false;

	float invDet = 1 / det;

	vec3 tvec = ray.origin() - v0.Position;
	float u = dot(tvec, pvec) * invDet;
	if (u < 0 || u > 1) return false;

	vec3 qvec = cross(tvec, v0v1);
	float v = dot(ray.direction(), qvec) * invDet;
	if (v < 0 || u + v > 1) return false;

	float temp = dot(v0v2, qvec) * invDet;
	if (temp < t_max && temp > t_min)
	{
		record.t = temp;
		record.p = ray.point_at_parameter(record.t);
		record.normal = cross(v0v1, v0v2);
		record.normal.make_unit_vector();
		record.mat_ptr = m_material;
		vec3 out_tex_coords = v0.TexCoord * (1 - u - v) + v1.TexCoord * u + v2.TexCoord * v;
		record.u = out_tex_coords.e[0];
		record.v = out_tex_coords.e[1];
		return true;
	}
	return false;

}
#endif // !MODELH
