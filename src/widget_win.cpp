#include <time.h>

#include "widget_win.h"

Widget* widget = NULL;

//������������¼�
static LRESULT CALLBACK widget_events(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) 
	{
	case WM_CLOSE: widget->widget_exit = 1; break;
	case WM_KEYDOWN: widget->widget_keys[wParam & 511] = 1; break;
	case WM_KEYUP: widget->widget_keys[wParam & 511] = 0; break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	//screen_keys[VK_ESCAPE] == 0
	return 0;
}

//������Ϣѭ������
void widget_dispatch() 
{
	MSG msg;
	while (1) 
	{
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}

	//�������̲���
// 	if (screen_keys[VK_UP]) pos += 0.01f;
// 	if (screen_keys[VK_DOWN]) pos -= 0.01f;
// 	if (screen_keys[VK_LEFT]) rotation += 0.01f;
// 	if (screen_keys[VK_RIGHT]) rotation -= 0.01f;
// 	if (screen_keys[VK_SPACE]) {
// 		int rd = renderer.render_type;
// 		if (rd < RENDER_TYPE_TEXTURE)
// 			rd = rd << 1;
// 		else
// 			rd = RENDER_TYPE_WIREFRAME;
// 		renderer.render_type = rd;
// 		screen_keys[VK_SPACE] = 0;
// 	}
// 	if (screen_keys[VK_F1]) {
// 		renderer.sampleTag++;
// 		screen_keys[VK_F1] = 0;
// 	}
// 	if (screen_keys[VK_F2]) {
// 		renderer.lightTag++;
// 		screen_keys[VK_F2] = 0;
// 	}
// 	if (screen_keys[VK_F3]) {
// 		renderer.normalTextureTag++;
// 		screen_keys[VK_F3] = 0;
// 	}
// 	if (screen_keys[VK_F4]) {
// 		renderer.modelTag++;
// 		renderer.modelTag = renderer.modelTag % modelList.size();
// 		if (renderer.modelTag)
// 			pos = 100;
// 		else
// 			pos = 3;
// 		currentModel = &modelList.at(renderer.modelTag);
// 		//���õ�ǰ����
// 		renderer.texture = &currentModel->texture;
// 		renderer.normalTexture = &currentModel->normalTexture;
// 		screen_keys[VK_F4] = 0;
// 	}
}

//�رմ���
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

//��ʼ������
int widget_init(int width, int height, const TCHAR* title) 
{
	if (widget)
	{
		widget_destroy();
		delete widget;
	}

	widget = new Widget();

	//���崰��
	WNDCLASSEX wc = WNDCLASSEX();
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpszClassName = title;
	wc.style = CS_BYTEALIGNCLIENT;
	wc.lpfnWndProc = widget_events;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	//ע�ᴰ��
	if (!RegisterClassEx(&wc)) return -1;

	//��������
	widget->widget_handle = CreateWindow(title, title, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (widget->widget_handle == NULL) return -2;

	//��ͼ�����ʼ��
	HDC temp_hdc = GetDC(widget->widget_handle);
	widget->widget_dc = CreateCompatibleDC(temp_hdc);//��ȡ�뵱ǰ�豸���ݵ������Ļ���
	ReleaseDC(widget->widget_handle, temp_hdc);
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB, (DWORD)(width * height * sizeof(Color)), 0, 0, 0, 0 } };//�����豸�޹�λͼ
	void* ptr;
	widget->widget_hb = CreateDIBSection(widget->widget_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);//�����豸�޹�λͼ
	if (widget->widget_hb == NULL) return -3;

	widget->widget_ob = (HBITMAP)SelectObject(widget->widget_dc, widget->widget_hb);//���豸���������滻HBITMAP���͵Ķ���
	widget->widget_fbuffer = (Color*)ptr;
	widget->widget_w = width;
	widget->widget_h = height;

	//������С
	RECT rect = { 0, 0, width, height };
	AdjustWindowRect(&rect, GetWindowLong(widget->widget_handle, GWL_STYLE), 0);

	//����λ��
	int wx, wy, sx, sy;
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(widget->widget_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));

	//��ʾ����
	SetForegroundWindow(widget->widget_handle);
	ShowWindow(widget->widget_handle, SW_NORMAL);
	widget_dispatch();

	memset(widget->widget_keys, 0, sizeof(int) * 512);
	//memset(widget->widget_fbuffer, 0xffffff, sizeof(Color) * width * height);
	widget->widget_exit = 0;

	return 0;
}

//���廭��ˢ��
void widget_tick() 
{
	HDC temp_hdc = GetDC(widget->widget_handle);
	BitBlt(temp_hdc, 0, 0, widget->widget_w, widget->widget_h, widget->widget_dc, 0, 0, SRCCOPY);
	ReleaseDC(widget->widget_handle, temp_hdc);
	//widget_dispatch();

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

//����fps
int cal_fps(long dur) 
{
	//ƽ��������
	static double avgDur = 0.0;
	static double alpha = 0.01;

	int fps = 0;
	if (dur == 0)
		avgDur = 0;
	else
		avgDur = avgDur * (1 - alpha) + dur * alpha;
	fps = (int)(1.0 / avgDur * CLOCKS_PER_SEC + 0.5);

	return Clamp(fps, 0, 1000);
}