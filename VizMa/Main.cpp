#pragma once
#define UNICODE // so windows knows what to take
#include <windows.h>

// creating a class name for our window
constexpr wchar_t CLASS_NAME[] = L"this is MY CLASS!";


void OnSize(HWND hwnd, UINT flag, int width, int height)
{

}

// Callback thst DispatchMessage calls once this windowproc is assigned to the window
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
		OnSize(hwnd, (UINT)wParam, width, height);
		break;
	
	}



	// default protocal
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// hInstance is basically program id, prev instance irrelevant is garbage from old windows, 
// pCmdLine = parameters in the command used to launch app, ncmdShow=number of the show state cmd requested
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// creating our window's class. Basically a template for what protocol our window uses, what instance handle
	WNDCLASSW wc = {};
	wc.lpfnWndProc = WindowProc; // protocol used (function that defines the windows behavior)
	wc.hInstance = hInstance; // handle of applicaiton that owns the class 
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc); // tells windows this class exists and to remeber it

	HWND hwnd = CreateWindowEx(
		0, // defualt stlye
		CLASS_NAME,
		L"My Window Name!!!",
		WS_OVERLAPPEDWINDOW,

		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	
	MSG msg = {};
	
	// Get Message is = 0 if the message is WM_QUIT
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);

	}

	return 0;
}



