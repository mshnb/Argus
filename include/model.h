#pragma once

#include "tools.h"

struct Vertex
{
	unsigned int iPostion, iNormal, iTexcoord;
};

struct Triangle
{
	Vertex vList[3];
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
	std::vector<vec3> vNormals;
	std::vector<vec2> vTexcoords;
	std::vector<Triangle> vTriangles;
};
