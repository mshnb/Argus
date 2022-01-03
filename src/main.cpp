#include <tchar.h>
#include <time.h>
#include <string>

#include "tools.h"
#include "renderer.h"
#include "widget_win.h"
#include "camera.h"

int parse_arg(int argc, char** argv);
void keyboard_input(float deltaTime = 0.0f);
void mouse_input();

unsigned int widget_width = 1024;
unsigned int widget_height = 768;

Renderer* renderer = NULL;

int main(int argc, char** argv)
{
	CHECK(parse_arg(argc, argv));

#ifdef UNICODE
	const TCHAR* widget_title = L"Argus Renderer";
#else
	const TCHAR* widget_title = "Argus Renderer";
#endif

	//create widget
	CHECK(widget_init(widget_width, widget_height, widget_title));

	//init renderer
	renderer = new Renderer(widget_width, widget_height);
	renderer->bindFrameBuffer(widget->widget_fbuffer);
	//renderer->isScanlineZbufferAlgo = true;

	//load model
	renderer->loadCude();

	//time
	float lastFrame = 0.0f;
	float deltaTime = 0.0f;

	while (!widget->widget_exit)
	{
		clock_t start = clock();
		deltaTime = (float)start / CLOCKS_PER_SEC - lastFrame;
		widget_dispatch();
		keyboard_input(deltaTime);
		mouse_input();

		lastFrame = (float)start / CLOCKS_PER_SEC;

		//rendering
		renderer->clearDepthBuffer();
		renderer->clearFrameBuffer();
		renderer->draw();
		//renderer->drawByScanLine();
		clock_t end = clock();

		int dur = end - start;
		int fps = cal_fps(dur);
		widget_print("fps:" + std::to_string(fps));
		widget_tick();
	}

	delete renderer;
	return 0;
}

int parse_arg(int argc, char** argv)
{
	for (size_t i = 1; i < argc; i++) 
	{
		if (!strcmp(argv[i], "-w")) 
		{
			++i; if (i >= argc) { return -1; }
			widget_width = std::stoi(argv[i++]);
		}
		else if (!strcmp(argv[i], "-h")) 
		{
			++i; if (i >= argc) { return -1; }
			widget_height = std::stoi(argv[i++]);
		}
		else 
		{
			WARN("unknow command %s.", argv[i]);
			return -1;
		}
	}

	return 0;
}

void keyboard_input(float deltaTime)
{
	Camera* camera = renderer->camera;
	int* keys = widget->widget_keys;
	if (keys[VK_ESCAPE])
		widget->widget_exit = 1;

	if (GetKeyState(VK_LSHIFT) < 0)
		camera->MovementSpeed = camera_run_speed;
	else
		camera->MovementSpeed = camera_walk_speed;

	if (keys['W'])
		camera->ProcessKeyboard(FORWARD, deltaTime);
	if (keys['S'])
		camera->ProcessKeyboard(BACKWARD, deltaTime);
	if (keys['A'])
		camera->ProcessKeyboard(LEFT, deltaTime);
	if (keys['D'])
		camera->ProcessKeyboard(RIGHT, deltaTime);
	if (keys['E'])
		camera->ProcessKeyboard(UP, deltaTime);
	if (keys['Q'])
		camera->ProcessKeyboard(DOWN, deltaTime);

	//TODO
	if (keys[VK_SPACE])
	{
		renderer->rType = (renderer->rType + 1) % Renderer::RenderType::Count;
		keys[VK_SPACE] = 0;
	}
}

void mouse_input()
{
	static POINT current;
	static bool handle_pressed = false;
	Mouse& info = widget->mouse_info;

	if (info.left_pressed)
	{
		get_mouse_pos(current);
		if (!handle_pressed)
		{
			info.pos.x = current.x;
			info.pos.y = current.y;
			handle_pressed = true;
		}

		//moving
		int xoffset = current.x - info.pos.x;
		int yoffset = info.pos.y - current.y; // reversed since y-coordinates go from bottom to top

		info.pos.x = current.x;
		info.pos.y = current.y;

		renderer->camera->ProcessMouseMovement(xoffset, yoffset);
	}
	else //release
		handle_pressed = false;
}