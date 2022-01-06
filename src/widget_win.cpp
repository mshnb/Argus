#include <time.h>
#include "widget_win.h"

#include "imm.h"
#pragma comment(lib, "imm32.lib")

Widget* widget = NULL;

//处理窗体过程事件
static LRESULT CALLBACK widget_events(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
	case WM_CLOSE: 
		widget->widget_exit = 1;
		break;
	case WM_KEYDOWN: 
		if(wParam == VK_PROCESSKEY)
			wParam = ImmGetVirtualKey(hWnd);

		widget->widget_keys[wParam & 511] = 1;
		break;
	case WM_KEYUP: 
		widget->widget_keys[wParam & 511] = 0;
		break;
	case WM_LBUTTONDOWN:
		widget->mouse_info.left_pressed = true;
		break;
	case WM_LBUTTONUP:
		widget->mouse_info.left_pressed = false;
		break;

	case WM_RBUTTONDOWN:
		POINT temp;
		get_mouse_pos(temp);
		printf("test.mouse pos [%d, %d]\n", temp.x, temp.y);
		widget->mouse_info.left_pressed = true;
		break;
	case WM_RBUTTONUP:
		widget->mouse_info.left_pressed = false;
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

void widget_dispatch() 
{
	MSG msg;
	while (1) 
	{
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

void widget_destroy()
{
	if (widget->widget_dc) {
		if (widget->widget_ob) {
			SelectObject(widget->widget_dc, widget->widget_ob);
			widget->widget_ob = NULL;
		}
		DeleteDC(widget->widget_dc);
		widget->widget_dc = NULL;
	}
	if (widget->widget_hb) {
		DeleteObject(widget->widget_hb);
		widget->widget_hb = NULL;
	}
	if (widget->widget_handle) {
		CloseWindow(widget->widget_handle);
		widget->widget_handle = NULL;
	}
}

int widget_init(int width, int height, const TCHAR* title) 
{
	if (widget)
	{
		widget_destroy();
		delete widget;
	}

	widget = new Widget();

	//define window
	WNDCLASSEX wc = WNDCLASSEX();
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpszClassName = title;
	wc.style = CS_BYTEALIGNCLIENT;
	wc.lpfnWndProc = widget_events;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	//register window
	if (!RegisterClassEx(&wc)) return -1;

	//create window
	widget->widget_handle = CreateWindow(title, title, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (widget->widget_handle == NULL) return -2;

	//init render target
	HDC temp_hdc = GetDC(widget->widget_handle);
	widget->widget_dc = CreateCompatibleDC(temp_hdc);
	ReleaseDC(widget->widget_handle, temp_hdc);
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB, (DWORD)(width * height * sizeof(Color)), 0, 0, 0, 0 } };
	void* ptr;
	widget->widget_hb = CreateDIBSection(widget->widget_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (widget->widget_hb == NULL) return -3;

	widget->widget_ob = (HBITMAP)SelectObject(widget->widget_dc, widget->widget_hb);
	widget->widget_fbuffer = (Color*)ptr;
	widget->widget_w = width;
	widget->widget_h = height;

	RECT rect = { 0, 0, width, height };
	AdjustWindowRect(&rect, GetWindowLong(widget->widget_handle, GWL_STYLE), 0);

	//set pos
	int wx, wy, sx, sy;
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(widget->widget_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));

	//show window
	SetForegroundWindow(widget->widget_handle);
	ShowWindow(widget->widget_handle, SW_NORMAL);
	widget_dispatch();

	memset(widget->widget_keys, 0, sizeof(int) * 512);
	widget->widget_exit = 0;

	return 0;
}

//refresh window
void widget_tick() 
{
	HDC temp_hdc = GetDC(widget->widget_handle);
	BitBlt(temp_hdc, 0, 0, widget->widget_w, widget->widget_h, widget->widget_dc, 0, 0, SRCCOPY);
	ReleaseDC(widget->widget_handle, temp_hdc);

	//clear offset in y axis
	widget->current_text_offset = 0;
}

void widget_print(std::string content)
{

#ifdef UNICODE
	int buffer_len = MultiByteToWideChar(CP_ACP, 0, content.c_str(), -1, NULL, 0) - 1;
	TCHAR* wbuf = new TCHAR[buffer_len];
	MultiByteToWideChar(CP_ACP, 0, content.c_str(), -1, wbuf, buffer_len);
#else
	int buffer_len = content.length();
	TCHAR* wbuf = new TCHAR[buffer_len + 1];
	strcpy(wbuf, content.c_str());
#endif

	TextOut(widget->widget_dc, 15, widget->current_text_offset, wbuf, buffer_len);
	widget->current_text_offset += 15;
	delete[] wbuf;
}

int cal_fps(long dur) 
{
	//average sample
	static double avgDur = 0.0;
	static double alpha = 0.01;

	int fps = 0;
	if (dur == 0)
		avgDur = 1;
	else
		avgDur = avgDur * (1 - alpha) + dur * alpha;
	fps = (int)(1.0 / avgDur * CLOCKS_PER_SEC + 0.5);

	return Clamp(fps, 0, 1000);
}

void get_mouse_pos(POINT& p)
{
	GetCursorPos(&p);
	ScreenToClient(widget->widget_handle, &p);
}