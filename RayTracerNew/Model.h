#pragma once
#include "hittable.h"
#include "material.h"
//#include "vec3.h"

struct Vertex
{
	vec3 Position;
	vec3 Normal;
	vec3 TexCoord;
	vec3 Color;
};

class Model
{

	bool LoadModel(std::string _modelPath, std::string file, std::unique_ptr<material> mat);
	bool hit(const ray& ray, float t_min, float t_max, hit_record& record)const;
	bool rayTriangleIntersect(const ray& ray, float t_min, float t_max, hit_record& record, const Vertex& v0, const Vertex& v1, const Vertex& v2)const;


	aabb m_bounding_box;
	std::unique_ptr<material> m_material;
	std::vector<Vertex> m_model;
	//material* m_material;
	
};

