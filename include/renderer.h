#pragma once
#include "tools.h"

class Camera;
class Model;

struct VertexData
{
	vec4 position;
	vec2 screen_pos;
};

// data structures for scan line zbuffer algorithm
struct EdgeNode
{
	int polygon_id;
	// small(up) y point's x
	float x;
	// large(down) y point's x
	float other_x;
	// the difference of intersection's x of 2 adject scan lines with this edge; same as 1/k
	float dx;
	// num of scan lines that edge cover
	int cover_y;
	EdgeNode* next = NULL;
};

struct PolygonNode
{
	int polygon_id;
	// plane equation
	float a, b, c, d;
	// depth of the vertex having smallest y
	float depth;
	PolygonNode* next = NULL;
};

struct ActiveEdgeNode
{
	//left intersection's x
	float x_left;
	// the difference of intersection's x of 2 adject scan lines with left edge
	float dx_left;
	// end point's x
	float end_x_left;
	// current index of scan line that left edge cover
	int cover_y_left;

	// same as left edge
	float x_right;
	float dx_right;
	float end_x_right;
	int cover_x_right;
	int cover_y_right;

	// polygon's depth in left
	float depth_left;
	// -a/c
	float depth_dx;
	// b/c
	float depth_dy;
	// the polygon that owns this left and right edges
	int polygon_id = -1;
	PolygonNode* polygon = NULL;
	ActiveEdgeNode* next = NULL;
};
// data structures end

class Renderer
{
public:
	Renderer() = delete;
	Renderer(int w, int h);

	~Renderer();

	void bindFrameBuffer(Color* fb);
	void clearFrameBuffer();

	void loadCude();
	void loadModel(std::string path);
	void draw();

	void resetCamera();
	void moveCamera(int direction, float deltaTime);
	void rotateCamera(int xoffset, int yoffset);

private:
	int widget_width;
	int widget_height;
	int buffer_length;

	bool isMvpDirty;
	Color* framebuffer = NULL;
	Color background = 0x00ffffff;

	vec3 bounding_min;
	vec3 bounding_max;

	glm::mat4 mModel;
	glm::mat4 mView;
	glm::mat4 mProjection;
	glm::mat4 pv;

	//memory pool start
	PolygonNode* polygonNodePool = NULL;
	EdgeNode* edgeNodePool = NULL;
	ActiveEdgeNode* activeEdgeNodePool = NULL;

	inline PolygonNode* getPolygonNode();
	inline EdgeNode* getEdgeNode();
	inline ActiveEdgeNode* getActiveEdgeNode();

	inline void releasePolygonNode(PolygonNode* p);
	inline void releaseEdgeNode(EdgeNode* p);
	inline void releaseActiveEdgeNode(ActiveEdgeNode* p);
	// memory pool end

	inline void calScreenPos(VertexData& v);
	inline int clip(vec4& clip_pos);

	inline void insertPolygon(int y, PolygonNode* polygon);
	inline void insertEdge(int y, EdgeNode* edge);
	inline void insertActiveEdge(ActiveEdgeNode* edge);

	// 1/width, 1/height
	float doubleWidthInv, doubleHeightInv;

	//zbuffer used in every frame
	float* scanline = NULL;
public:
	enum RenderType
	{
		Normal,
		Depth,
		Count
	};

	// scanline zbuffer
	std::vector<PolygonNode*> polygonTable;
	std::vector<EdgeNode*> edgeTable;

	ActiveEdgeNode* activeEdgeList = NULL;

	int rType = RenderType::Normal;
	Camera* camera = NULL;
	std::vector<Model> vModels;
};