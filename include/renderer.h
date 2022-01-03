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

// data structures for scan line zbuffer algorithm

struct EdgeNode
{
	int model_id;
	int polygon_id;
	// small(up) y point's x
	float x;
	// the difference of intersection's x of 2 adject scan lines with this edge; same as 1/k
	float dx;
	// num of scan lines that edge cover
	int dy;
	EdgeNode* next = NULL;
};


struct PolygonNode
{
	int model_id;
	int polygon_id;
	// plane equation
	float a, b, c, d;
	bool shading_flag;
	// num of scan lines that polygon cover
	int dy;
	// depth of the vertex having smallest y
	float depth;
	PolygonNode* next = NULL;
};

struct ActiveEdgeNode
{
	//left intersection's x
	int x_left;
	// the difference of intersection's x of 2 adject scan lines with left edge
	int dx_left;
	// current index of scan line that left edge cover
	int dy_left;

	// same as left edge
	int x_right;
	int dx_right;
	int dy_right;

	// polygon's depth in left
	float depth_left;
	// -a/c
	float depth_dx;
	// b/c
	float depth_dy;
	// the polygon that owns this left and right edges
	int polygon_id = -1;
};

// data structures end

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
	void drawByScanLine();

private:
	int widget_width;
	int widget_height;
	int buffer_length;

	int current_model = -1;
	bool isMvpDirty = false;
	float* zbuffer = NULL;
	Color* framebuffer = NULL;
	Color background = 0x00ffffff;

	glm::mat4 mModel;
	glm::mat4 mView;
	glm::mat4 mProjection;
	glm::mat4 pv;
	//glm::mat4 mTbn;

	inline void drawPixel(int x, int y, Color color);
	//bresenham
	void drawLine(vec2& p1, vec2& p2, Color color);
	void drawTriangle(VertexData& v1, VertexData& v2, VertexData& v3);
	void drawScanLine(VertexData& v1, VertexData& v2);
	void calScreenPos(VertexData& v);
	int clip(vec4& clip_pos);

public:
	enum RenderType
	{
		Wireframe,
		Texture,
		Depth,
		Texcoords,
		Count
	};

	// scanline zbuffer
	bool isScanlineZbufferAlgo = false;
	std::vector<PolygonNode*> polygonTable;
	std::vector<EdgeNode*> edgeTable;

	std::vector<PolygonNode*> activePolygonTable;
	std::vector<ActiveEdgeNode*> activeEdgeTable;

	int rType = RenderType::Wireframe;
	Camera* camera = NULL;
	std::vector<Model> vModels;
};