#pragma once

#include "tools.h"

struct Texture 
{
	unsigned char* data;
	int channels;
	int texType;
	int width, height;
};

struct Vertex
{
	int iPostion, iNormal, iTexcoord;
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

	Model();
	Model(std::string& name);

//private:
	Texture* albedo = NULL;
	//Texture* normal = NULL;

	int nVertex;
	std::vector<glm::vec3> vPositions;
	std::vector<glm::vec3> vNormals;
	std::vector<glm::vec2> vTexcoords;
	std::vector<Triangle> vTriangles;
};
