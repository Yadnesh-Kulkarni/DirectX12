#include "stdafx.h"
#include "D3D12HelloWorld.h"

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR,int iCmdShow)
{
	D3D12HelloWorld helloWin(1280, 768, L"Plain Window");
	return BaseWindowClass::Start(&helloWin);
}
