#pragma once
#define UNICODE // so windows knows what to take
#include <windows.h>

const wchar_t* CLASS_NAME = L"this is MY CLASS!";

WNDCLASS wc = {}
wc.lpfnWndProc = WindowProc;
wc.hInstance = hInstance;
wc.lpszClassName = CLASS_NAME;

RegisterClass(&wc);