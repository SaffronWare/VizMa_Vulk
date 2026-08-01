#pragma once
#include <Windows.h>
#include <stdexcept>
#include "DEBUG.h"

constexpr const char* WINDOW_CLASS_NAME = "Window Class";


class Window
{
private:

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	Window(HINSTANCE hInstance, int nCmdShow, const char* app_title);
	~Window();

	int loop();

	HWND getWindowHandle() const;
	HINSTANCE getWindowInstance() const;

private:
	static void RegisterWindowClass(HINSTANCE hinstance);

	static WNDCLASS wndclass;
	static bool class_registered;
	
	HINSTANCE hinstance = NULL;
	HWND hwnd = NULL;

	const char* wnd_title;
};