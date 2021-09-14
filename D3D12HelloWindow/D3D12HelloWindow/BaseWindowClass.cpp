
#include "stdafx.h"
#include "BaseWindowClass.h"

#define WIDTH 1024
#define HEIGHT 768

HWND BaseWindowClass::m_hwnd = nullptr;

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

	m_hwnd = CreateWindow(
		wndclass.lpszClassName,
		pGraphicsClass->GetTitle(),
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
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
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	
	return DefWindowProc(hwnd,iMsg,wParam,lParam);
}
