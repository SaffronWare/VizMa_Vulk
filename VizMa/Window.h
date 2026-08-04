#pragma once
#include <Windows.h>
#include <stdexcept>
#include "DEBUG.h"
#include <optional>


constexpr const char* WINDOW_CLASS_NAME = "Window Class";


class Window
{
private:

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


public:
	Window(HINSTANCE hInstance, int nCmdShow, const char* app_title);
	~Window();

	bool loop();

	//bool updateClientSurfaceRect();
	int getClientSurfaceWidth() const;
	int getClientSurfaceHeight() const;

	HWND getWindowHandle() const;
	HINSTANCE getWindowInstance() const;

	

private:
	static void RegisterWindowClass(HINSTANCE hinstance);

	static std::optional<WNDCLASS> wndclass;
	
	HINSTANCE hinstance = NULL;
	HWND hwnd = NULL;

	const char* wnd_title;
};