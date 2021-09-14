#pragma once

#include "GraphicsBase.h"

class GraphicsBase;

class BaseWindowClass
{
public:
	static int Start(GraphicsBase*);
	static HWND GetHwnd() { return m_hwnd; }
	static void ToggleFullscreen();
protected:
	static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);

private:
	static HWND m_hwnd;
	static bool m_Fullscreen;
	static DWORD m_dwStyle;
	static WINDOWPLACEMENT m_wpPrev;
};

