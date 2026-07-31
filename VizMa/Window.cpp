#include "Window.h"

bool Window::class_registered = false;
WNDCLASS Window::wndclass = static_cast<WNDCLASS>(0);


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int height;
	int width;
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		width = LOWORD(lParam);
		height = HIWORD(lParam);
		//OnSize(hwnd, (UINT)wParam, width, height);
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Window::RegisterWindowClass(HINSTANCE hinstance)
{
	if (!class_registered)
	{
		wndclass.hInstance = hinstance;
		wndclass.lpfnWndProc = WindowProc;
		wndclass.lpszClassName = WINDOW_CLASS_NAME;
		RegisterClass(&wndclass);
		class_registered = true;
	}
}

Window::Window(HINSTANCE hInstance, int nCmdShow, const char* app_title) :
	hinstance(hInstance),
	wnd_title(app_title)
{
	RegisterWindowClass(hInstance);

	hwnd = CreateWindowEx(
		0,
		WINDOW_CLASS_NAME,
		app_title,
		WS_OVERLAPPEDWINDOW,

		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hwnd == NULL)
	{
		throw std::runtime_error("FAILED TO CREATE WINDOW\n");
	}

	ShowWindow(hwnd, nCmdShow);
}

Window::~Window()
{

}

int Window::loop()
{
	MSG msg;

	while (true)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);

			if (msg.message == WM_QUIT)
			{
				return 0;
			}
		}

	}
}

HWND Window::getWindowHandle() const
{
	return hwnd;
}

HINSTANCE Window::getWindowInstance() const
{
	return hinstance;
}
