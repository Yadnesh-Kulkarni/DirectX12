#include "stdafx.h"
#include "D3D12HelloWorld.h"

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR,int iCmdShow)
{
	D3D12HelloTexture helloWin(1280, 720, L"Plain Window");
	return BaseWindowClass::Start(&helloWin);
}
