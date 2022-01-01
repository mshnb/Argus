#include "renderer.h"
#include "camera.h"
#include "model.h"

Renderer::Renderer(int w, int h)
{
	widget_width = w;
	widget_height = h;
	buffer_length = w * h;
	zbuffer = new float[buffer_length];

	mModel = glm::mat4(1.0f);
	mView = glm::mat4(1.0f);
	mProjection = glm::mat4(1.0f);

	camera = new Camera();
	clearDepthBuffer();

	//TODO
	camera->Position = glm::vec3(1.0f, 1.0f, 1.0f);
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
		 1.0f, 1.0f,
		 0.0f, 1.0f
	};
	cube.vTexcoords.resize(static_cast<size_t>(4));
	memcpy(&*cube.vTexcoords.begin(), texcoords, 4 * 2 * sizeof(float));

	cube.vTriangles.push_back(Triangle{ Vertex{0, 5, 0}, Vertex{5, 5, 1}, Vertex{4, 5, 3} });
	cube.vTriangles.push_back(Triangle{ Vertex{0, 5, 0}, Vertex{1, 5, 2}, Vertex{5, 5, 1} });

	cube.vTriangles.push_back(Triangle{ Vertex{2, 2, 0}, Vertex{7, 2, 1}, Vertex{6, 2, 3} });
	cube.vTriangles.push_back(Triangle{ Vertex{2, 2, 0}, Vertex{3, 2, 2}, Vertex{7, 2, 1} });

	cube.vTriangles.push_back(Triangle{ Vertex{4, 1, 0}, Vertex{6, 1, 1}, Vertex{7, 1, 3} });
	cube.vTriangles.push_back(Triangle{ Vertex{4, 1, 0}, Vertex{5, 1, 2}, Vertex{6, 1, 1} });

	cube.vTriangles.push_back(Triangle{ Vertex{3, 4, 0}, Vertex{1, 4, 1}, Vertex{0, 4, 3} });
	cube.vTriangles.push_back(Triangle{ Vertex{3, 4, 0}, Vertex{2, 4, 2}, Vertex{1, 4, 1} });

	cube.vTriangles.push_back(Triangle{ Vertex{1, 0, 0}, Vertex{6, 0, 1}, Vertex{5, 0, 3} });
	cube.vTriangles.push_back(Triangle{ Vertex{1, 0, 0}, Vertex{2, 0, 2}, Vertex{6, 0, 1} });

	cube.vTriangles.push_back(Triangle{ Vertex{3, 3, 0}, Vertex{4, 3, 1}, Vertex{7, 3, 3} });
	cube.vTriangles.push_back(Triangle{ Vertex{3, 3, 0}, Vertex{0, 3, 2}, Vertex{4, 3, 1} });
}

void Renderer::draw()
{
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
				vd[k].texcoord = model.vTexcoords[v.iTexcoord];
			}
			drawTriangle(vd[0], vd[1], vd[2]);
		}
	}
}

inline void Renderer::drawPixel(int x, int y, Color& color)
{
	framebuffer[y * widget_width + x] = color;
}

void Renderer::drawLine(vec2& p1, vec2& p2, Color& color)
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
		for (; x != p2.x; x += ux)
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
		for (; y != p2.y; y += uy)
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
void Renderer::pos2Screen(VertexData& v)
{
	float w = v.clip_pos.w;
	if (abs(w) < 1e-3) return;

	//透视除法
	float onePerW = 1.0f / w;
	v.screen_pos = v.clip_pos * onePerW;

	//转化至屏幕坐标
	v.screen_pos.x = (v.screen_pos.x + 1.0f) * 0.5f * widget_width;
	v.screen_pos.y = (1.0f - v.screen_pos.y) * 0.5f * widget_height;
}

int Renderer::clip(vec4& clip_pos)
{
	float w = clip_pos.w;//原来的z
	if (clip_pos[0] > w || clip_pos[0] < -w) return 1;
	if (clip_pos[1] > w || clip_pos[1] < -w) return 1;
	if (clip_pos[2] > w || clip_pos[2] < 0.0f) return 1;
	return 0;
}

void Renderer::drawTriangle(VertexData& v1, VertexData& v2, VertexData& v3)
{
	//calculate world pos
	v1.world_pos = mModel * v1.position;
	v2.world_pos = mModel * v2.position;
	v3.world_pos = mModel * v3.position;

	//背面消隐 正面看该三角形时 顶点顺序需满足 v1,v2,v3呈逆时针
	if (rType != RenderType::Wireframe) 
	{
		vec3 u(v2.world_pos - v1.world_pos), v(v3.world_pos - v1.world_pos);
		glm::vec3 norl = glm::cross(u, v), view = camera->Position - v1.world_pos.xyz();
		float ans = glm::dot(norl, view);
		if (ans > 0) return;
	}

	//TODO
	//世界坐标->相机坐标->齐次裁剪坐标
	v1.clip_pos = mProjection * mView * v1.world_pos;
	v2.clip_pos = mProjection * mView * v2.world_pos;
	v3.clip_pos = mProjection * mView * v3.world_pos;

	//粗略的裁剪 
	if (clip(v1.clip_pos) || clip(v2.clip_pos) || clip(v3.clip_pos)) return;

	//齐次裁剪坐标->屏幕坐标
	pos2Screen(v1);
	pos2Screen(v2);
	pos2Screen(v3);

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
		//TODO
		// 
		//使用世界矩阵的逆矩阵的转置调整法线
// 		t1.normal = t1.normal * transform.worldInvTranspose;
// 		t2.normal = t2.normal * transform.worldInvTranspose;
// 		t3.normal = t3.normal * transform.worldInvTranspose;

/*
		onePerW = position[3] == 0.0f ? 1.0f : 1.0f / position[3];
		u *= onePerW;
		v *= onePerW;

		worldP.m_array[0] *= onePerW;
		worldP.m_array[1] *= onePerW;
		worldP.m_array[2] *= onePerW;

		normal.m_array[0] *= onePerW;
		normal.m_array[1] *= onePerW;
		normal.m_array[2] *= onePerW;
*/

// 		t1.init();
// 		t2.init();
// 		t3.init();
// 
// 		//对顶点进行y坐标排序
// 		Vertex temp;
// 		if (t1.position[1] > t2.position[1]) {
// 			temp = t2;
// 			t2 = t1;
// 			t1 = temp;
// 		}
// 		if (t2.position[1] > t3.position[1]) {
// 			temp = t3;
// 			t3 = t2;
// 			t2 = temp;
// 		}
// 		if (t1.position[1] > t2.position[1]) {
// 			temp = t2;
// 			t2 = t1;
// 			t1 = temp;
// 		}
// 
// 		float dp12, dp13;//反向斜率
// 		if (t2.position[1] - t1.position[1] > 0)
// 			dp12 = (t2.position[0] - t1.position[0]) / (t2.position[1] - t1.position[1]);
// 		else
// 			dp12 = 0;
// 		if (t3.position[1] - t1.position[1] > 0)
// 			dp13 = (t3.position[0] - t1.position[0]) / (t3.position[1] - t1.position[1]);
// 		else
// 			dp13 = 0;
// 
// 		Vertex vleft, vright;
// 		float g1, g2;
// 
// 		for (int y = (int)(t1.position[1] + 0.5f); y < t3.position[1]; y++)
// 		{
// 			g1 = t1.position[1] == t3.position[1] ? 1 : (y - t1.position[1]) / (t3.position[1] - t1.position[1]);
// 			vleft.interpolation(t1, t3, g1);
// 			if (y < t2.position[1]) {
// 				g2 = t1.position[1] == t2.position[1] ? 1 : (y - t1.position[1]) / (t2.position[1] - t1.position[1]);
// 				vright.interpolation(t1, t2, g2);
// 			}
// 			else {
// 				g2 = t2.position[1] == t3.position[1] ? 1 : (y - t2.position[1]) / (t3.position[1] - t2.position[1]);
// 				vright.interpolation(t2, t3, g2);
// 			}
// 			vleft.position.m_array[1] = (float)y;
// 			vright.position.m_array[1] = (float)y;
// 
// 			if (dp12 > dp13) //case1:p2在p13右侧
// 				drawScanLine(vleft, vright);
// 			else             //case2:p2在p13左侧
// 				drawScanLine(vright, vleft);
// 		}
	}
}
