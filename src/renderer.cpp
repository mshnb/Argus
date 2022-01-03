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
	memcpy(&*cube.vPositions.begin(), positions, 8 * 3 * sizeof(float));

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
	memcpy(&*cube.vNormals.begin(), normals, 6 * 3 * sizeof(float));

	float texcoords[] =
	{
		 0.0f, 0.0f,
		 1.0f, 0.0f,
		 0.0f, 1.0f,
		 1.0f, 1.0f
	};
	cube.vTexcoords.resize(static_cast<size_t>(4));
	memcpy(&*cube.vTexcoords.begin(), texcoords, 4 * 2 * sizeof(float));

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
}

void Renderer::draw()
{
	//TODO
	mView = camera->GetViewMatrix();

	static VertexData vd[3];
	for (int i = 0; i < vModels.size(); i++)
	{
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
}

inline void Renderer::drawPixel(int x, int y, Color color)
{
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
	glm::mat4 pv = mProjection * mView;
	v1.position = pv * v1.position;
	v2.position = pv * v2.position;
	v3.position = pv * v3.position;

	//粗略的裁剪 
	if (clip(v1.position) || clip(v2.position) || clip(v3.position)) return;

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

	for (int x = round(x1); x < x2; x++) 
	{
		float g = x1 == x2 ? 1.0f : (x - x1) / (x2 - x1);
		float depth = Interp(v1.position.w, v2.position.w, g);
		if (y < 0 || y >= widget_height || x < 0 || x >= widget_width)
			continue;

		if (zbuffer[y * widget_width + x] < depth)
		{
			float u = (float)Interp(v1.texcoord.x, v2.texcoord.x, g);
			float v = (float)Interp(v1.texcoord.y, v2.texcoord.y, g);

			//0x00ff0000
			Color visual = ((int)(255 * u) << 8) + (int)(255 * v);
			drawPixel(x, y, visual); //readTexture(u, v)

			//update zbuffer
			zbuffer[y * widget_width + x] = depth;
		}
	}
}