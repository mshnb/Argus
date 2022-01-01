#pragma once
#include "tools.h"

class Camera;
class Model;

struct VertexData
{
	vec4 position;
	vec3 normal;
	vec2 texcoord;

	vec4 world_pos;
	vec4 clip_pos;
	vec2 screen_pos;
};

class Renderer
{
	enum RenderType
	{
		Wireframe,
		Polygon
	};

public:
	Renderer() = delete;
	Renderer(int w, int h);

	~Renderer();

	void bindFrameBuffer(Color* fb);
	void clearDepthBuffer();
	void clearFrameBuffer();

	void loadCude();
	void loadModel(std::string& path);
	void draw();

private:
	int widget_width;
	int widget_height;
	int buffer_length;

	bool isMvpDirty = false;
	float* zbuffer = NULL;
	Color* framebuffer = NULL;

	Color background = 0x00ffffff;

	glm::mat4 mModel;
	glm::mat4 mView;
	glm::mat4 mProjection;
	//glm::mat4 mTbn;

	inline void drawPixel(int x, int y, Color& color);
	//bresenham
	void drawLine(vec2& p1, vec2& p2, Color& color);
	void drawTriangle(VertexData& v1, VertexData& v2, VertexData& v3);

	void pos2Screen(VertexData& v);
	int clip(vec4& clip_pos);

public:
	RenderType rType = RenderType::Wireframe;
	Camera* camera = NULL;
	std::vector<Model> vModels;
};