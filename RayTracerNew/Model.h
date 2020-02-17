#pragma once
#include "material.h"
#include "vec3.h"

class Model
{
	bool init(std::string _modelPath);
	bool LoadModel(std::string _modelPath, std::string file, std::unique_ptr<material> mat);


	std::unique_ptr<material> m_material;
	std::vector<Vertex> m_model;
	//material* m_material;
	
};

