#pragma once

#include "tools.h"

struct Texture 
{
	std::string sName;
	unsigned char* data;
	int channels;
	int texType;
	int width, height;

	Color read(float u, float v);
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
	~Model();

	void loadTexture(std::string path);
	Color readAlbedo(float u, float v);
//private:
	Texture* albedo = NULL;
	//Texture* normal = NULL;

	int nVertex;
	std::vector<vec3> vPositions;
	std::vector<vec3> vNormals;
	std::vector<vec2> vTexcoords;
	std::vector<Triangle> vTriangles;
};
