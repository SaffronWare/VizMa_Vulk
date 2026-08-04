#include "Window.h"

std::optional<WNDCLASS> Window::wndclass = std::nullopt;

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	if (!wndclass.has_value())
	{
		wndclass.emplace();
		wndclass.value().hInstance = hinstance;
		wndclass.value().lpfnWndProc = WindowProc;
		wndclass.value().lpszClassName = WINDOW_CLASS_NAME;
		RegisterClass(&wndclass.value());
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

bool Window::loop()
{
	MSG msg;


	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);

		if (msg.message == WM_QUIT)
		{
			return true;
		}
	}

	return false;
	
}

int Window::getClientSurfaceWidth() const
{
	RECT SurfaceRect;
	GetClientRect(hwnd, &SurfaceRect);
	return SurfaceRect.right - SurfaceRect.left;
}

int Window::getClientSurfaceHeight() const
{
	RECT SurfaceRect;
	GetClientRect(hwnd, &SurfaceRect);
	return SurfaceRect.bottom - SurfaceRect.top;
}

HWND Window::getWindowHandle() const
{
	return hwnd;
}

HINSTANCE Window::getWindowInstance() const
{
	return hinstance;
}
