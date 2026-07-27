#pragma once
#define UNICODE // so windows knows what to take
#include <windows.h>

// creating a class name for our window
constexpr wchar_t CLASS_NAME[] = L"this is MY CLASS!";

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
		WS_OVERLAPPED_WINDOW,

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

	ShowWindow(hwnd, nCmdShow)
}


