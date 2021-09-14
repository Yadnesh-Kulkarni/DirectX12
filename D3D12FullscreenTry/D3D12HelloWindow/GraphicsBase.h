#pragma once

#include "BaseWindowClass.h"
#include "GraphicsBaseHelper.h"


class GraphicsBase
{
public:
	GraphicsBase(UINT width, UINT height, std::wstring name);
	virtual ~GraphicsBase();

	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;
	virtual void OnSizeChanged(UINT width,UINT height) = 0;

	//Event Handlers To Handle Specific Messages
	virtual void OnKeyDown(UINT) {}
	virtual void OnKeyUp(UINT) {}
	virtual void UpdateSize(UINT width,UINT height);

	void LogToFile(const char*);

	//Getters
	UINT GetWidth() const		{ return m_width; }
	UINT GetHeight() const		{ return m_height; }
	const WCHAR* GetTitle() const { return m_title.c_str(); }

protected:
	std::wstring GetAssetFullPath(LPCWSTR assetName);
	void GetHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
	void SetCustomWindowText(LPCWSTR text);

	//Viewport Dimensions
	UINT m_width;
	UINT m_height;
	float m_aspectRatio;

	bool m_useWarpDevice;

private:
	std::wstring m_assetPath;
	std::wstring m_title;
	FILE* m_file;
};

