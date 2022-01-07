#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "renderer.h"
#include "camera.h"
#include "model.h"

Renderer::Renderer(int w, int h)
{
	widget_width = w;
	widget_height = h;
	buffer_length = w * h;

	mModel = glm::mat4x4(1.0f);
	mView = glm::mat4x4(1.0f);
	mProjection = glm::mat4(1.0f);

	camera = new Camera();
	camera->Position = glm::vec3(0.0f, 0.0f, 3.0f);
	mView = camera->GetViewMatrix();
	mProjection = glm::perspective(glm::radians(camera->Zoom), (float)w / (float)h, camera->tNear, camera->tFar);

	polygonTable.resize(widget_height);
	edgeTable.resize(widget_height);

	memset(&polygonTable[0], 0, widget_height * sizeof(void*));
	memset(&edgeTable[0], 0, widget_height * sizeof(void*));

	scanline = new float[widget_width];
	doubleWidthInv = 2.0f / widget_width;
	doubleHeightInv = 2.0f / widget_height;

	bounding_min = vec3(FLT_MAX);
	bounding_max = vec3(-FLT_MAX);

	isMvpDirty = true;
}

Renderer::~Renderer() 
{
	if (scanline)
		delete[] scanline;
	scanline = NULL;

	for (int i = 0; i < polygonTable.size(); i++)
	{
		auto p = polygonTable[i];
		while (p)
		{
			auto next = p->next;
			delete p;
			p = next;
		}
	}
	polygonTable.clear();

	for (int i = 0; i < edgeTable.size(); i++)
	{
		auto p = edgeTable[i];
		while (p)
		{
			auto next = p->next;
			delete p;
			p = next;
		}
	}
	edgeTable.clear();

	{
		auto p = activeEdgeList;
		while (p)
		{
			auto next = p->next;
			delete p;
			p = next;
		}
		activeEdgeList = NULL;
	}

	if (camera)
		delete camera;
	camera = NULL;

	//clear mem pool
	{
		auto p = polygonNodePool;
		while (p)
		{
			auto next = p->next;
			delete p;
			p = next;
		}
		polygonNodePool = NULL;
	}

	{
		auto p = edgeNodePool;
		while (p)
		{
			auto next = p->next;
			delete p;
			p = next;
		}
		edgeNodePool = NULL;
	}

	{
		auto p = activeEdgeNodePool;
		while (p)
		{
			auto next = p->next;
			delete p;
			p = next;
		}
		activeEdgeNodePool = NULL;
	}
}

void Renderer::resetCamera()
{
	isMvpDirty = true;
	float radius = glm::length(bounding_max - bounding_min) * 0.5f;
	glm::vec3 bounding_center = (bounding_max + bounding_min) * 0.5f;

	camera->tNear = 0.1f * radius;
	camera->tFar = 2.5f * radius;
	camera->Position = bounding_center - 1.5f * radius * camera->Front;
	camera->camera_walk_speed = radius * 2.5f;
	camera->camera_run_speed = camera->camera_walk_speed * 3.0f;

	mProjection = glm::perspective(glm::radians(camera->Zoom), (float)widget_width / (float)widget_height, camera->tNear, camera->tFar);
}

void Renderer::moveCamera(int direction, float deltaTime)
{
	isMvpDirty = true;
	camera->ProcessKeyboard(direction, deltaTime);
}

void Renderer::rotateCamera(int xoffset, int yoffset)
{
	isMvpDirty = true;
	camera->ProcessMouseMovement(xoffset, yoffset);
}

void Renderer::bindFrameBuffer(Color* fb)
{
	assert(fb);
	if (framebuffer)
		delete[] framebuffer;

	framebuffer = fb;
	clearFrameBuffer();
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

	cube.vTriangles.push_back(Triangle{0, 5, 4});
	cube.vTriangles.push_back(Triangle{0, 1, 5});

	cube.vTriangles.push_back(Triangle{2, 7, 6});
	cube.vTriangles.push_back(Triangle{2, 3, 7});

	cube.vTriangles.push_back(Triangle{4, 6, 7});
	cube.vTriangles.push_back(Triangle{4, 5, 6});

	cube.vTriangles.push_back(Triangle{3, 1, 0});
	cube.vTriangles.push_back(Triangle{3, 2, 1});

	cube.vTriangles.push_back(Triangle{1, 6, 5});
	cube.vTriangles.push_back(Triangle{1, 2, 6});

	cube.vTriangles.push_back(Triangle{3, 4, 7});
	cube.vTriangles.push_back(Triangle{3, 0, 4});
}

void Renderer::loadModel(std::string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate); // | aiProcess_FlipUVs
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		WARN("Error occur in loading model:%s, error info:%s", path.c_str(), importer.GetErrorString());
		getchar();
		abort();
	}

	int numVerticesTotal = 0;
	int numFacesTotal = 0;

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* ai_mesh = scene->mMeshes[i];
		int numVertices = ai_mesh->mNumVertices;
		vModels.emplace_back();
		Model& model = vModels.back();

		model.vPositions.resize(static_cast<size_t>(numVertices));
		memcpy(&model.vPositions[0], ai_mesh->mVertices, numVertices * 3 * sizeof(float));

		int numFaces = ai_mesh->mNumFaces;
		model.vTriangles.reserve(numFaces);
		for (int j = 0; j < numFaces; j++)
		{
			aiFace& face = ai_mesh->mFaces[j];
			assert(face.mNumIndices == 3);
			model.vTriangles.push_back(Triangle{ face.mIndices[2], face.mIndices[1], face.mIndices[0] });
			
			//cal bounding box
			for (int k = 0; k < 3; k++)
			{
				vec3& v = model.vPositions[face.mIndices[k]];
				bounding_min.x = std::min(bounding_min.x, v.x);
				bounding_min.y = std::min(bounding_min.y, v.y);
				bounding_min.z = std::min(bounding_min.z, v.z);
				bounding_max.x = std::max(bounding_max.x, v.x);
				bounding_max.y = std::max(bounding_max.y, v.y);
				bounding_max.z = std::max(bounding_max.z, v.z);
			}
		}

		numVerticesTotal += numVertices;
		numFacesTotal += numFaces;
	}

	resetCamera();
	INFO("%s loaded with %d vertex and %d faces.", path.c_str(), numVerticesTotal, numFacesTotal);
}

void Renderer::draw()
{
	//temp data
	static VertexData vd[3];

	// clear y-bucket
	for (int i = 0; i < polygonTable.size(); i++)
	{
		while (polygonTable[i])
		{
			PolygonNode* next = polygonTable[i]->next;
			releasePolygonNode(polygonTable[i]);
			polygonTable[i] = next;
		}
	}
		
	for (int i = 0; i < edgeTable.size(); i++)
	{
		while (edgeTable[i])
		{
			EdgeNode* next = edgeTable[i]->next;
			releaseEdgeNode(edgeTable[i]);
			edgeTable[i] = next;
		}
	}

	// reduce computing
	if (isMvpDirty)
	{
		mView = camera->GetViewMatrix();
		pv = mProjection * mView;
		isMvpDirty = false;
	}

	int polygon_id = 0;
	glm::vec3 face_normal;
	for (int i = 0; i < vModels.size(); i++)
	{
		Model& model = vModels[i];
		for (int j = 0; j < model.vTriangles.size(); j++)
		{
			Triangle& t = model.vTriangles[j];
			for (int k = 0; k < 3; k++)
			{
				vd[k].position.x = model.vPositions[t.iPostion[k]].x;
				vd[k].position.y = model.vPositions[t.iPostion[k]].y;
				vd[k].position.z = model.vPositions[t.iPostion[k]].z;
				vd[k].position.w = 1.0f;
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

				{
					//face culling
					glm::vec3 u(v2.position - v1.position), v(v3.position - v1.position);
					face_normal = glm::cross(u, v);
					glm::vec3 view = camera->Position - v1.position.xyz();
					float ans = glm::dot(face_normal, view);
					if (ans > 0)
						continue;
				}

				//world pos to clip pos
				v1.position = pv * v1.position;
				v2.position = pv * v2.position;
				v3.position = pv * v3.position;

				//simple clip
				if (clip(v1.position) || clip(v2.position) || clip(v3.position)) continue;

				//ndc pos to screen pos
				calScreenPos(v1);
				calScreenPos(v2);
				calScreenPos(v3);
			}

			float depth = FLT_MAX;
			float min_y = FLT_MAX;
			float max_y = -FLT_MAX;

			// min_y point's x
			float min_y_x;

			// create edge table
			for (int k = 0; k < 3; k++)
			{
				VertexData* v1 = &vd[k];
				VertexData* v2 = &vd[(k + 1) % 3];

				//make v1.y < v2.y
				if (v1->screen_pos.y > v2->screen_pos.y)
					std::swap(v1, v2);

				int cover_y = floor(v2->screen_pos.y) - floor(v1->screen_pos.y);
				if (cover_y == 0) // ignore horizontal edge
					continue;

				EdgeNode* edge = getEdgeNode();
				edge->x = v1->screen_pos.x;
				edge->dx = (v2->screen_pos.x - v1->screen_pos.x) / (v2->screen_pos.y - v1->screen_pos.y);
				edge->other_x = v2->screen_pos.x;
				edge->cover_y = cover_y + 1;
				edge->polygon_id = polygon_id;

				//insert to edge table
				int y_idx = floor(v1->screen_pos.y);
				insertEdge(y_idx, edge);

				// use floor to avoid nearly horizontal edge leading to use wrong edge's depth
				if (floor(min_y) > y_idx || (floor(min_y) == y_idx && min_y_x > v1->screen_pos.x))
				{
					depth = v1->position.z;
					min_y = v1->screen_pos.y;
					min_y_x = v1->screen_pos.x;
				}

				max_y = std::max(max_y, v2->screen_pos.y);
			}

			//create polygon table
			int cover_y = floor(max_y) - floor(min_y);
			if (cover_y > 0 && max_y > 0 && min_y < widget_height)
			{
				PolygonNode* polygon = getPolygonNode();
				polygon->polygon_id = polygon_id++;

				//calculate normal
				glm::vec3 u(vd[2].position - vd[0].position), v(vd[1].position - vd[0].position);
				face_normal = glm::normalize(glm::cross(u, v));
				polygon->a = face_normal.x;
				polygon->b = face_normal.y;
				polygon->c = face_normal.z;
				polygon->d = -(face_normal.x * vd[0].position.x + face_normal.y * vd[0].position.y + face_normal.z * vd[0].position.z);

				polygon->depth = depth;
				int y_idx = floor(min_y);
				insertPolygon(y_idx, polygon);
			}
		}
	}

	// draw scan lines
	while (activeEdgeList)
	{
		ActiveEdgeNode* next = activeEdgeList->next;
		releaseActiveEdgeNode(activeEdgeList);
		activeEdgeList = next;
	}

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
					// edges from same polygon always insert continually
					e1 = edge;
					e2 = edge->next;
#ifdef _DEBUG
					if(!e2)
					{
						WARN("error occur when generating active edge, can't find 2 edge in y=%d.", y);
						abort();
					}
#endif
					break;
				}
				edge = edge->next;
			}

			if (e1->x == e2->x)
			{
				if(e1->dx > e2->dx)
					std::swap(e1, e2);
			}
			else if(e1->x > e2->x)
				std::swap(e1, e2);

			// e1 joint scan line first
			ActiveEdgeNode* active_edge = getActiveEdgeNode();
			active_edge->x_left = e1->x;
			active_edge->dx_left = e1->dx;
			active_edge->end_x_left = e1->other_x;
			active_edge->cover_y_left = e1->cover_y;

			active_edge->x_right = e2->x;
			active_edge->dx_right = e2->dx;
			active_edge->end_x_right = e2->other_x;
			active_edge->cover_y_right = e2->cover_y;

			active_edge->depth_left = polygon->depth;
			if (abs(polygon->c) < 1e-1)//TODO reduce artifacts
			{
				active_edge->depth_dx = 0.0f;
				active_edge->depth_dy = 0.0f;
			}
			else
			{
				float one_div_c = 1.0f / polygon->c;
				active_edge->depth_dx = -doubleWidthInv * polygon->a * one_div_c;
				active_edge->depth_dy = doubleHeightInv * polygon->b * one_div_c;
			}

			active_edge->polygon = polygon;
			active_edge->polygon_id = polygon->polygon_id;
			insertActiveEdge(active_edge);

			polygon = polygon->next;
		}

		// fill scan line
		ActiveEdgeNode* prev_edge = NULL;
		ActiveEdgeNode* active_edge = activeEdgeList;
		while (active_edge)
		{
			if (!(active_edge->x_left >= widget_width || active_edge->x_right < 0))
			{
				float depth = active_edge->depth_left;
				for (int x = (int)(active_edge->x_left); x <= (int)(active_edge->x_right); x++)
				{
					if (scanline[x] == 0 || scanline[x] > depth)
					{
						Color visual = 0x00000000;
						if (rType == RenderType::Depth)
						{
							int d = 255 * Clamp(depth, 0.0f, 1.0f);
							visual = (d << 16) + (d << 8) + d;
						}
						else if (rType == RenderType::Normal)
						{
							//[-1,1] -> [0, 1]
							int r = (active_edge->polygon->a + 1.0f) * 0.5f * 255;
							int g = (active_edge->polygon->b + 1.0f) * 0.5f * 255;
							int b = (active_edge->polygon->c + 1.0f) * 0.5f * 255;

							visual = (r << 16) + (g << 8) + b;
						}

						linebuffer[x] = visual;
						scanline[x] = depth;
					}

					depth += active_edge->depth_dx;
				}
			}

			//update to next line's active edge
			active_edge->cover_y_left -= 1;
			active_edge->cover_y_right -= 1;

			// check this final 2 pixel because some step maybe very large causing depth's discontinuity
			float real_x_step = active_edge->dx_left;
			if (active_edge->cover_y_left < 2)
			{
				float next_x_left = active_edge->x_left + active_edge->dx_left;
				if ((active_edge->dx_left < 0 && next_x_left < active_edge->end_x_left)
					|| (active_edge->dx_left > 0 && next_x_left > active_edge->end_x_left))
					real_x_step = active_edge->end_x_left - active_edge->x_left;
			}

			// update depth for next scan line
			active_edge->depth_left += real_x_step * active_edge->depth_dx + active_edge->depth_dy;

			//change edge or remove active edges pair
			if (active_edge->cover_y_left == 0 || active_edge->cover_y_right == 0)
			{
				if (active_edge->cover_y_left == 0 && active_edge->cover_y_right == 0)
				{
					// remove this active edges pair
					ActiveEdgeNode* next = active_edge->next;
					releaseActiveEdgeNode(active_edge) ;
					if (!prev_edge)
						activeEdgeList = next;
					else
						prev_edge->next = next;
					active_edge = next;
					continue;
				}

				// remove one of active-edge-pair's edge and add the new one
				EdgeNode* edge = edgeTable[y];
				while (edge)
				{
					if (edge->polygon_id == active_edge->polygon_id)
						break;
					edge = edge->next;
				}
#ifdef _DEBUG
				if (!edge)
				{
					WARN("error occur when change to another active edge, can't find that edge in y=%d.", y);
					abort();
				}
#endif
				// set new steps and remain lines num
				if (active_edge->cover_y_left == 0)
				{
					active_edge->x_left = edge->x;
					active_edge->dx_left = edge->dx;
					active_edge->end_x_left = edge->other_x;
					active_edge->cover_y_left = edge->cover_y - 1;
				}
				else 
				{
					active_edge->x_right = edge->x;
					active_edge->dx_right = edge->dx;
					active_edge->end_x_right = edge->other_x;
					active_edge->cover_y_right = edge->cover_y - 1;
				}
			}

			//update point's x for left and right edges
			active_edge->x_left += active_edge->dx_left;
			active_edge->x_right += active_edge->dx_right;

			// some step maybe too large
			if (active_edge->cover_y_left == 1 && ((active_edge->dx_left < 0 && active_edge->x_left < active_edge->end_x_left)
				|| (active_edge->dx_left > 0 && active_edge->x_left > active_edge->end_x_left)))
				active_edge->x_left = active_edge->end_x_left;

			if (active_edge->cover_y_right == 1 && ((active_edge->dx_right < 0 && active_edge->x_right < active_edge->end_x_right)
				|| (active_edge->dx_right > 0 && active_edge->x_right > active_edge->end_x_right)))
				active_edge->x_right = active_edge->end_x_right;

			prev_edge = active_edge;
			active_edge = active_edge->next;
		}
	}
}

void Renderer::calScreenPos(VertexData& v)
{
	float w = v.position.w;
	if (abs(w) < 1e-4) return;

	//perspective division
	float onePerW = 1.0f / w;
	v.position = v.position * onePerW;

	//save depth info for depth test
	//v.position.w = onePerW;
	
	//ndc to screen pos
	v.screen_pos.x = (v.position.x + 1.0f) * 0.5f * (widget_width - 1.0f);
	v.screen_pos.y = (1.0f - v.position.y) * 0.5f * (widget_height - 1.0f);
}

int Renderer::clip(vec4& clip_pos)
{
	//ndc's z
	float w = clip_pos.w;
	if (clip_pos.x > w || clip_pos.x < -w) return 1;
	if (clip_pos.y > w || clip_pos.y < -w) return 1;
	if (clip_pos.z > w || clip_pos.z < 0.0f) return 1;
	return 0;
}

void Renderer::insertPolygon(int y, PolygonNode* polygon)
{
	polygon->next = polygonTable[y];
	polygonTable[y] = polygon;
}

void Renderer::insertEdge(int y, EdgeNode* edge)
{
	edge->next = edgeTable[y];
	edgeTable[y] = edge;
}

void Renderer::insertActiveEdge(ActiveEdgeNode* edge)
{
	edge->next = activeEdgeList;
	activeEdgeList = edge;
}

PolygonNode* Renderer::getPolygonNode()
{
	if (polygonNodePool)
	{
		auto p = polygonNodePool;
		polygonNodePool = polygonNodePool->next;
		p->next = NULL;
		return p;
	}
	else
		return new PolygonNode();
}

EdgeNode* Renderer::getEdgeNode() 
{
	if (edgeNodePool)
	{
		auto p = edgeNodePool;
		edgeNodePool = edgeNodePool->next;
		p->next = NULL;
		return p;
	}
	else
		return new EdgeNode();
}

ActiveEdgeNode* Renderer::getActiveEdgeNode()
{
	if (activeEdgeNodePool)
	{
		auto p = activeEdgeNodePool;
		activeEdgeNodePool = activeEdgeNodePool->next;
		p->next = NULL;
		return p;
	}
	else
		return new ActiveEdgeNode();
}

void Renderer::releasePolygonNode(PolygonNode* p)
{
	p->next = polygonNodePool;
	polygonNodePool = p;
}

void Renderer::releaseEdgeNode(EdgeNode* p)
{
	p->next = edgeNodePool;
	edgeNodePool = p;
}

void Renderer::releaseActiveEdgeNode(ActiveEdgeNode* p)
{
	p->next = activeEdgeNodePool;
	activeEdgeNodePool = p;
}

