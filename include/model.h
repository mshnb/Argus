#pragma once

#include "tools.h"

struct Triangle
{
	unsigned int iPostion[3];
};

class Model 
{
public:
	std::string sName;
	glm::mat4x4 mTransform;

	Model() {};
	~Model() {};

	int nVertex;
	std::vector<vec3> vPositions;
	std::vector<Triangle> vTriangles;
};
