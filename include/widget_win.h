#pragma once

#include <windows.h>

#include "tools.h"

struct Mouse
{
	vec2 pos;
	bool left_pressed = false;
	bool right_pressed = false;
	float wheel_delta = 0.0f;
};

struct Widget
{
	HWND widget_handle = NULL;		// 主窗口 HWND
	HDC widget_dc = NULL;			// 设备上下文环境句柄
	HBITMAP widget_hb = NULL;		// 位图句柄
	HBITMAP widget_ob = NULL;		// 被取代的旧位图句柄

	//unsigned char* window_fb;
	Color* widget_fbuffer = NULL;		// 帧缓存
	unsigned int widget_w;
	unsigned int widget_h;
	int widget_keys[512];

	int widget_exit = 0;

	Mouse mouse_info;
	int current_text_offset = 0;
};

extern Widget* widget;

int widget_init(int width, int height, const TCHAR* title);
void widget_tick();
void widget_dispatch();
void widget_destroy();

void widget_print(std::string content);

int cal_fps(long dur);
void get_mouse_pos(POINT& p);