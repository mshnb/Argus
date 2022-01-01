#include <tchar.h>
#include <time.h>
#include <string>


#include "tools.h"
#include "renderer.h"
#include "widget_win.h"

int parse_arg(int argc, char** argv);

unsigned int widget_width = 1920;
unsigned int widget_height = 1080;

int main(int argc, char** argv)
{
	CHECK(parse_arg(argc, argv));

#ifdef UNICODE
	const TCHAR* widget_title = L"Argus Renderer";
#else
	const TCHAR* widget_title = "Argus Renderer";
#endif

	CHECK(widget_init(widget_width, widget_height, widget_title));

	Renderer renderer(widget_width, widget_height);
	renderer.bindFrameBuffer(widget->widget_fbuffer);

	renderer.loadCude();

// 	renderer.resetCamera(3, 0, 0);
// 
// 	loadModel("box");
// 
// 	currentModel = &modelList.at(0);
// 	renderer.texture = &currentModel->texture;
// 	renderer.normalTexture = &currentModel->normalTexture;

	while (!widget->widget_exit)
	{
		widget_dispatch();
		clock_t start = clock();

		//rendering
		renderer.clearDepthBuffer();
		renderer.clearFrameBuffer();
		renderer.draw();

		clock_t end = clock();
		int dur = end - start;
		int fps = cal_fps(dur);
		widget_print("fps:" + std::to_string(fps));
		widget_tick();
	}

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