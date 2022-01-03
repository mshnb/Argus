#include "renderer.h"
#include "camera.h"
#include "model.h"

Renderer::Renderer(int w, int h)
{
	widget_width = w;
	widget_height = h;
	buffer_length = w * h;
	zbuffer = new float[buffer_length];

	mModel = glm::mat4x4(1.0f);
	mView = glm::mat4x4(1.0f);
	mProjection = glm::mat4(1.0f);

	camera = new Camera();
	clearDepthBuffer();

	//TODO
	camera->Position = glm::vec3(-3.0f, 0.0f, 0.0f);
	mView = camera->GetViewMatrix();
	mProjection = glm::perspective(glm::radians(camera->Zoom), (float)w / (float)h, 0.1f, 100.0f);

	polygonTable.resize(widget_height);
	edgeTable.resize(widget_height);

	memset(&polygonTable[0], 0, widget_height * sizeof(void*));
	memset(&edgeTable[0], 0, widget_height * sizeof(void*));
}

Renderer::~Renderer() 
{
	if (zbuffer)
		delete[] zbuffer;
	zbuffer = NULL;
}

void Renderer::bindFrameBuffer(Color* fb)
{
	assert(fb);
	if (framebuffer)
		delete[] framebuffer;

	framebuffer = fb;
	clearFrameBuffer();
}

void Renderer::clearDepthBuffer()
{
	assert(zbuffer);
	memset(zbuffer, 0.0f, sizeof(float) * buffer_length);
}

void Renderer::clearFrameBuffer()
{
	assert(framebuffer);
	memset(framebuffer, background, sizeof(Color) * buffer_length);
}

void Renderer::loadCude()
{
	vModels.emplace_back();
	Model& cube = vModels.back();

	float positions[] = 
	{
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f
	};
	cube.vPositions.resize(static_cast<size_t>(8));
	memcpy(&cube.vPositions[0], positions, 8 * 3 * sizeof(float));

	float normals[] =
	{
		 1.0f, 0.0f, 0.0f,
		 0.0f, 1.0f, 0.0f,
		 0.0f, 0.0f, 1.0f,
		-1.0f, 0.0f, 0.0f,
		 0.0f,-1.0f, 0.0f,
		 0.0f, 0.0f,-1.0f
	};
	cube.vNormals.resize(static_cast<size_t>(6));
	memcpy(&cube.vNormals[0], normals, 6 * 3 * sizeof(float));

	float texcoords[] =
	{
		 0.0f, 0.0f,
		 1.0f, 0.0f,
		 0.0f, 1.0f,
		 1.0f, 1.0f
	};
	cube.vTexcoords.resize(static_cast<size_t>(4));
	memcpy(&cube.vTexcoords[0], texcoords, 4 * 2 * sizeof(float));

	cube.vTriangles.push_back(Triangle{ Vertex{0, 5, 0}, Vertex{5, 5, 3}, Vertex{4, 5, 2} });
	cube.vTriangles.push_back(Triangle{ Vertex{0, 5, 0}, Vertex{1, 5, 1}, Vertex{5, 5, 3} });

	cube.vTriangles.push_back(Triangle{ Vertex{2, 2, 0}, Vertex{7, 2, 3}, Vertex{6, 2, 2} });
	cube.vTriangles.push_back(Triangle{ Vertex{2, 2, 0}, Vertex{3, 2, 1}, Vertex{7, 2, 3} });

	cube.vTriangles.push_back(Triangle{ Vertex{4, 1, 0}, Vertex{6, 1, 3}, Vertex{7, 1, 2} });
	cube.vTriangles.push_back(Triangle{ Vertex{4, 1, 0}, Vertex{5, 1, 1}, Vertex{6, 1, 3} });

	cube.vTriangles.push_back(Triangle{ Vertex{3, 4, 0}, Vertex{1, 4, 3}, Vertex{0, 4, 2} });
	cube.vTriangles.push_back(Triangle{ Vertex{3, 4, 0}, Vertex{2, 4, 1}, Vertex{1, 4, 3} });

	cube.vTriangles.push_back(Triangle{ Vertex{1, 0, 0}, Vertex{6, 0, 3}, Vertex{5, 0, 2} });
	cube.vTriangles.push_back(Triangle{ Vertex{1, 0, 0}, Vertex{2, 0, 1}, Vertex{6, 0, 3} });

	cube.vTriangles.push_back(Triangle{ Vertex{3, 3, 0}, Vertex{4, 3, 3}, Vertex{7, 3, 2} });
	cube.vTriangles.push_back(Triangle{ Vertex{3, 3, 0}, Vertex{0, 3, 1}, Vertex{4, 3, 3} });

	cube.loadTexture("../resource/frog.jpg");
}

void Renderer::drawByScanLine()
{
	static VertexData vd[3];

	memset(&polygonTable[0], 0, widget_height * sizeof(void*));
	memset(&edgeTable[0], 0, widget_height * sizeof(void*));

	mView = camera->GetViewMatrix();
	pv = mProjection * mView;

	for (int i = 0; i < vModels.size(); i++)
	{
		current_model = i;
		Model& model = vModels[i];
		for (int j = 0; j < model.vTriangles.size(); j++)
		{
			Triangle& t = model.vTriangles[j];
			for (int k = 0; k < 3; k++)
			{
				Vertex& v = t.vList[k];

				vd[k].position.x = model.vPositions[v.iPostion].x;
				vd[k].position.y = model.vPositions[v.iPostion].y;
				vd[k].position.z = model.vPositions[v.iPostion].z;
				vd[k].position.w = 1.0f;

				vd[k].normal = model.vNormals[v.iNormal];
				vd[k].normal.w = 0.0f;

				vd[k].texcoord = model.vTexcoords[v.iTexcoord];
			}

			// local pos to screen pos
			{
				VertexData& v1 = vd[0];
				VertexData& v2 = vd[1];
				VertexData& v3 = vd[2];

				//local pos to world pos
				v1.position = mModel * v1.position;
				v2.position = mModel * v2.position;
				v3.position = mModel * v3.position;

				//背面消隐 正面看该三角形时 顶点顺序需满足 v1,v2,v3呈逆时针
				if (rType != RenderType::Wireframe)
				{
					vec3 u(v2.position - v1.position), v(v3.position - v1.position);
					glm::vec3 norl = glm::cross(u, v), view = camera->Position - v1.position.xyz();
					float ans = glm::dot(norl, view);
					if (ans > 0)
						continue;
				}

				//world pos to clip pos
				v1.position = pv * v1.position;
				v2.position = pv * v2.position;
				v3.position = pv * v3.position;

				//clip
				// if (clip(v1.position) || clip(v2.position) || clip(v3.position)) return;

				//clip pos to screen pos
				calScreenPos(v1);
				calScreenPos(v2);
				calScreenPos(v3);
			}

			float depth = 0.0f;
			float min_y = FLT_MAX;
			float max_y = -FLT_MAX;

			// create edge table
			for (int k = 0; k < 3; k++)
			{
				VertexData& v1 = vd[k];
				VertexData& v2 = vd[(k + 1) % 3];

				//make v1.y < v2.y
				if (v1.screen_pos.y > v2.screen_pos.y)
					std::swap(v1, v2);

				int cover_y = round(v2.screen_pos.y) - round(v1.screen_pos.y);
				if (cover_y == 0) // horizontal edge
					continue;

				EdgeNode* edge = new EdgeNode();
				edge->x = v1.screen_pos.x;
				edge->dx = (v2.screen_pos.x - v1.screen_pos.x) / (v2.screen_pos.y - v1.screen_pos.y);
				edge->dy = cover_y;
				edge->model_id = current_model;
				edge->polygon_id = j;

				//insert to edge table
				int y_idx = round(v1.screen_pos.y);
				if (edgeTable[y_idx])
					edgeTable[y_idx]->next = edge;
				else
					edgeTable[y_idx] = edge;


				if (min_y > v1.screen_pos.y)
				{
					depth = v1.position.w;
					min_y = v1.screen_pos.y;
				}
				max_y = std::max(max_y, v2.screen_pos.y);
			}

			//create polygon table
			int cover_y = round(max_y) - round(min_y);
			if (cover_y > 0 && max_y > 0 && min_y < widget_height)
			{
				PolygonNode* polygon = new PolygonNode();
				polygon->model_id = current_model;
				polygon->polygon_id = j;
				polygon->dy = cover_y;

				//calculate normal
				vec3 u(vd[1].position - vd[0].position), v(vd[2].position - vd[0].position);
				glm::vec3 normal = glm::cross(u, v);
				polygon->a = normal.x;
				polygon->b = normal.y;
				polygon->c = normal.z;
				polygon->d = -(normal.x * vd[0].position.x + normal.y * vd[0].position.y + normal.z * vd[0].position.z);

				polygon->depth = depth;
				int y_idx = round(min_y);
				if (polygonTable[y_idx])
					polygonTable[y_idx]->next = polygon;
				else
					polygonTable[y_idx] = polygon;
			}
		}
	}

	current_model = -1;

	// draw scan lines
	static float* scanline = NULL;
	if (scanline == NULL)
		scanline = new float[widget_width];

	activeEdgeTable.clear();
	activePolygonTable.clear();

	//scan y top to bottom
	for (int y = 0; y < widget_height; y++)
	{
		memset(scanline, 0, widget_width * sizeof(float));
		Color* linebuffer = &framebuffer[y * widget_width];

		PolygonNode* polygon = polygonTable[y];
		while (polygon)
		{
			EdgeNode* e1 = NULL;
			EdgeNode* e2 = NULL;
			EdgeNode* edge = edgeTable[y];
			while (edge)
			{
				if (edge->polygon_id == polygon->polygon_id)
				{
					if (!e1)
						e1 = edge;
					else if (!e2)
						e2 = edge;
					else
					{
						WARN("error occur when generating active edge, 3 edges joint at same point.");
						abort();
					}
				}
				edge = edge->next;
			}

			if (!e1 || !e2)
			{
				WARN("error occur when generating active edge, can't find 2 edge.");
				abort();
			}

			int x1_left = e1->dx > 0 ? e1->x : (e1->x + e1->dx * e1->dy);
			int x2_left = e2->dx > 0 ? e2->x : (e2->x + e2->dx * e2->dy);
			if (x1_left > x2_left)
			{
				std::swap(x1_left, x2_left);
				std::swap(e1, e2);
			}

			// e1 joint scan line first
			ActiveEdgeNode* active_edge = new ActiveEdgeNode();
			active_edge->x_left = x1_left;
			active_edge->dx_left = e1->dx;
			active_edge->dy_left = e1->dy;

			active_edge->x_right = x2_left;
			active_edge->dx_right = e2->dx;
			active_edge->dy_right = e2->dy;

			//TODO 
			active_edge->depth_left = polygon->depth;
			active_edge->depth_dx = -polygon->a / polygon->c;
			active_edge->depth_dy = polygon->b / polygon->c;

			active_edge->polygon_id = polygon->polygon_id;
			activeEdgeTable.push_back(active_edge);

			activePolygonTable.push_back(polygon);
			polygon = polygon->next;
		}

		//TODO




	}
}

void Renderer::draw()
{
	static VertexData vd[3];
	//TODO
	mView = camera->GetViewMatrix();
	pv = mProjection * mView;

	for (int i = 0; i < vModels.size(); i++)
	{
		current_model = i;
		Model& model = vModels[i];
		for(int j = 0; j < model.vTriangles.size(); j++)
		{
			Triangle& t = model.vTriangles[j];
			for (int k = 0; k < 3; k++)
			{
				Vertex& v = t.vList[k];
				
				vd[k].position.x = model.vPositions[v.iPostion].x;
				vd[k].position.y = model.vPositions[v.iPostion].y;
				vd[k].position.z = model.vPositions[v.iPostion].z;
				vd[k].position.w = 1.0f;

				vd[k].normal = model.vNormals[v.iNormal];
				vd[k].normal.w = 0.0f;

				vd[k].texcoord = model.vTexcoords[v.iTexcoord];
			}

			drawTriangle(vd[0], vd[1], vd[2]);
		}
	}

	current_model = -1;
}

inline void Renderer::drawPixel(int x, int y, Color color)
{
	if (x < 0 || x >= widget_width || y < 0 || y >= widget_height)
		return;
	framebuffer[y * widget_width + x] = color;
}

void Renderer::drawLine(vec2& p1, vec2& p2, Color color)
{
	int dx = p2.x - p1.x;
	int dy = p2.y - p1.y;
	int ux = ((dx > 0) << 1) - 1;	//x的增量方向
	int uy = ((dy > 0) << 1) - 1;	//y的增量方向
	int x = p1.x, y = p1.y, eps = 0;	//eps为累计误差

	dx = abs(dx);
	dy = abs(dy);

	if (dx > dy) 
	{
		for (; x != (int)p2.x; x += ux)
		{
			drawPixel(x, y, color);
			eps += dy;
			if (!((eps << 1) < dx)) 
			{
				y += uy;
				eps -= dx;
			}
		}
	}
	else {
		for (; y != (int)p2.y; y += uy)
		{
			drawPixel(x, y, color);
			eps += dx;
			if (!((eps << 1) < dy)) 
			{
				x += ux;
				eps -= dy;
			}
		}
	}
}

//TODO
void Renderer::calScreenPos(VertexData& v)
{
	float w = v.position.w;
	if (abs(w) < 1e-4) return;

	//透视除法
	float onePerW = 1.0f / w;
	v.position = v.position * onePerW;
	v.position.w = onePerW;//save depth info for depth test

	//转化至屏幕坐标
	v.screen_pos.x = (v.position.x + 1.0f) * 0.5f * widget_width;
	v.screen_pos.y = (1.0f - v.position.y) * 0.5f * widget_height;
}

int Renderer::clip(vec4& clip_pos)
{
	float w = clip_pos.w;//原来的z
	if (clip_pos.x > w || clip_pos.x < -w) return 1;
	if (clip_pos.y > w || clip_pos.y < -w) return 1;
	if (clip_pos.z > w || clip_pos.z < 0.0f) return 1;
	return 0;
}

void Renderer::drawTriangle(VertexData& v1, VertexData& v2, VertexData& v3)
{
	//local pos to world pos
	v1.position = mModel * v1.position;
	v2.position = mModel * v2.position;
	v3.position = mModel * v3.position;

	//背面消隐 正面看该三角形时 顶点顺序需满足 v1,v2,v3呈逆时针
	if (rType != RenderType::Wireframe) 
	{
		vec3 u(v2.position - v1.position), v(v3.position - v1.position);
		glm::vec3 norl = glm::cross(u, v), view = camera->Position - v1.position.xyz();
		float ans = glm::dot(norl, view);
		if (ans > 0) return;
	}

	//world pos to clip pos
	v1.position = pv * v1.position;
	v2.position = pv * v2.position;
	v3.position = pv * v3.position;

	//粗略的裁剪 
	// if (clip(v1.position) || clip(v2.position) || clip(v3.position)) return;

	//clip pos to screen pos
	calScreenPos(v1);
	calScreenPos(v2);
	calScreenPos(v3);

	//绘制线框
	if (rType == RenderType::Wireframe) 
	{
		Color wire_color = 0x000000ff;
		drawLine(v1.screen_pos, v2.screen_pos, wire_color);
		drawLine(v2.screen_pos, v3.screen_pos, wire_color);
		drawLine(v3.screen_pos, v1.screen_pos, wire_color);
	}
	else
	{

// 		glm::mat4 world_inverse_transpose = glm::transpose(glm::inverse(mModel));
// 		v1.normal = world_inverse_transpose * v1.normal;
// 		v2.normal = world_inverse_transpose * v2.normal;
// 		v3.normal = world_inverse_transpose * v3.normal;

		//sort by y to get v1.y < v2.y < v3.y
		if (v1.screen_pos.y > v2.screen_pos.y)
			std::swap(v1, v2);
		if (v1.screen_pos.y > v3.screen_pos.y)
			std::swap(v1, v3);
		if (v2.screen_pos.y > v3.screen_pos.y)
			std::swap(v2, v3);

		VertexData left, right;
		int top_y = floor(v3.screen_pos.y);
		int mid_y = floor(v2.screen_pos.y);
		int bottom_y = floor(v1.screen_pos.y);

		//bottom part
		if (mid_y > bottom_y)
		{
			for (int y = bottom_y + 1; y <= mid_y; y++)
			{
				left.interp(v1, v3, (y - v1.screen_pos.y) / (v3.screen_pos.y - v1.screen_pos.y));
				right.interp(v1, v2, (y - v1.screen_pos.y) / (v2.screen_pos.y - v1.screen_pos.y));
				left.screen_pos.y = y;
				right.screen_pos.y = y;

				if (left.screen_pos.x < right.screen_pos.x)
					drawScanLine(left, right);
				else
					drawScanLine(right, left);
			}
		}

		//top part
		if (top_y > mid_y)
		{
			for (int y = mid_y + 1; y <= top_y; y++)
			{
				left.interp(v1, v3, (y - v1.screen_pos.y) / (v3.screen_pos.y - v1.screen_pos.y));
				right.interp(v2, v3, (y - v2.screen_pos.y) / (v3.screen_pos.y - v2.screen_pos.y));
				left.screen_pos.y = y;
				right.screen_pos.y = y;

				if (left.screen_pos.x < right.screen_pos.x)
					drawScanLine(left, right);
				else
					drawScanLine(right, left);
			}
		}
	}
}

void Renderer::drawScanLine(VertexData& v1, VertexData& v2)
{
	int y = v1.screen_pos.y;
	float x1 = v1.screen_pos.x, x2 = v2.screen_pos.x;

	for (int x = floor(x1) + 1; x <= x2; x++) 
	{
		float g = (x2 - x1 < 1e-4) ? 1.0f : (x - x1) / (x2 - x1);
		float depth = Interp(v1.position.w, v2.position.w, g);
		if (y < 0 || y >= widget_height || x < 0 || x >= widget_width)
			continue;

		if (zbuffer[y * widget_width + x] < depth)
		{
			float u = (float)Interp(v1.texcoord.x, v2.texcoord.x, g);
			float v = (float)Interp(v1.texcoord.y, v2.texcoord.y, g);

			if (rType == RenderType::Texture)
			{
				Color pixel_color = vModels[current_model].readAlbedo(u, v);
				drawPixel(x, y, pixel_color);
			}
			else if (rType == RenderType::Depth)
			{
				int d = 255 * Clamp(depth, 0.0f, 1.0f);
				Color visual = (d << 16) + (d << 8) + d;
				drawPixel(x, y, visual);
			}
			else if (rType == RenderType::Texcoords)
			{
				Color visual = ((int)(255 * u) << 8) + (int)(255 * v);
				drawPixel(x, y, visual);
			}

			//update zbuffer
			zbuffer[y * widget_width + x] = depth;
		}
	}
}