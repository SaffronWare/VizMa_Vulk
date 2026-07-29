#pragma once
#include <Windows.h>
#include <stdexcept>

constexpr wchar_t* WINDOW_CLASS_NAME = L"Window Class";


class Window
{
private:

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	Window(HINSTANCE hInstance, int nCmdShow, const wchar_t* app_title);
	~Window();

	int loop();

	HWND getWindowHandle();
	HINSTANCE getWindowInstance();

private:
	static void RegisterWindowClass();

	static WNDCLASS wndclass;
	static bool class_registered;
	
	HINSTANCE hinstance = NULL;
	HWND hwnd = NULL;

	const wchar_t* wnd_title;
};