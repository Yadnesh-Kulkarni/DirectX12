
#include "stdafx.h"
#include "BaseWindowClass.h"

#define WIDTH 1024
#define HEIGHT 768

HWND BaseWindowClass::m_hwnd = nullptr;
bool BaseWindowClass::m_Fullscreen = false;
DWORD BaseWindowClass::m_dwStyle = 0;
WINDOWPLACEMENT BaseWindowClass::m_wpPrev = {};

int BaseWindowClass::Start(GraphicsBase* pGraphicsClass)
{
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	
	//Send To Graphics Class for Command Line Parsing

	LocalFree(argv);

	WNDCLASSEX wndclass = { 0 };
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = GetModuleHandle(NULL);
	wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	wndclass.lpszClassName = TEXT("GraphicsSampleClass");
	RegisterClassEx(&wndclass);

	RECT windowRect = { 0, 0, static_cast<LONG>(pGraphicsClass->GetWidth()), static_cast<LONG>(pGraphicsClass->GetHeight()) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	m_hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		wndclass.lpszClassName,
		pGraphicsClass->GetTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		pGraphicsClass->GetWidth(),
		pGraphicsClass->GetHeight(),
		NULL,
		NULL,
		GetModuleHandle(NULL),
		pGraphicsClass
	);

	//Initialize Sample Object
	pGraphicsClass->OnInit();

	ShowWindow(m_hwnd, SW_SHOW);

	MSG msg = {};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//Destroy Sample Object
	pGraphicsClass->OnDestroy();


	return static_cast<char>(msg.wParam);
}

LRESULT BaseWindowClass::WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	GraphicsBase* pGraphicsBase = reinterpret_cast<GraphicsBase*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));
	
	switch (iMsg)
	{
	case WM_CREATE:
		{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;
	case WM_KEYDOWN:
		if (pGraphicsBase)
		{
			if (static_cast<UINT8>(wParam) == 0x46)
			{
				ToggleFullscreen();
			}
			pGraphicsBase->OnKeyDown(static_cast<UINT8>(wParam));
		}
		return 0;
	case WM_KEYUP:
		if (pGraphicsBase)
		{
			pGraphicsBase->OnKeyUp(static_cast<UINT8>(wParam));
		}
		return 0;
	case WM_PAINT:
		if (pGraphicsBase)
		{
			pGraphicsBase->OnUpdate();
			pGraphicsBase->OnRender();
		}
		return 0;
	//case WM_SIZE:
	//	pGraphicsBase->LogToFile("Calling WM_SIZE\n");
	//	if (pGraphicsBase)
	//	{
	//		//pGraphicsBase->OnSizeChanged(LOWORD(lParam),HIWORD(lParam));
	//	}
	//	return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	
	return DefWindowProc(hwnd,iMsg,wParam,lParam);
}

void BaseWindowClass::ToggleFullscreen(void)
{
	MONITORINFO mi;

	if (m_Fullscreen == false)
	{
		m_dwStyle = GetWindowLong(GetHwnd(), GWL_STYLE);
		if (m_dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(GetHwnd(), &m_wpPrev) && GetMonitorInfo(MonitorFromWindow(GetHwnd(), MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(GetHwnd(), GWL_STYLE, m_dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(GetHwnd(), HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
	}
	else
	{
		SetWindowLong(GetHwnd(), GWL_STYLE, m_dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(GetHwnd(), &m_wpPrev);
		SetWindowPos(GetHwnd(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
}
