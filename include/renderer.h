#pragma once
#include "tools.h"

class Camera;
class Model;

struct VertexData
{
	vec4 position;
	vec4 normal;
	vec2 texcoord;

	vec2 screen_pos;

	void interp(VertexData& a, VertexData& b, float alpha)
	{
		//position.interp(a.position, b.position, alpha);
		position.w = Interp(a.position.w, b.position.w, alpha);//TODO
		normal.interp(a.normal, b.normal, alpha);
		texcoord.interp(a.texcoord, b.texcoord, alpha);
		screen_pos.interp(a.screen_pos, b.screen_pos, alpha);
	}
};

class Renderer
{
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

	inline void drawPixel(int x, int y, Color color);
	//bresenham
	void drawLine(vec2& p1, vec2& p2, Color color);
	void drawTriangle(VertexData& v1, VertexData& v2, VertexData& v3);
	void Renderer::drawScanLine(VertexData& v1, VertexData& v2);

	void pos2Screen(VertexData& v);
	int clip(vec4& clip_pos);

public:
	enum RenderType
	{
		Wireframe,
		Polygon,
		Count
	};
	int rType = RenderType::Wireframe;
	Camera* camera = NULL;
	std::vector<Model> vModels;
};