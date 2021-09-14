#include "stdafx.h"
#include "GraphicsBase.h"

using namespace Microsoft::WRL;

GraphicsBase::GraphicsBase(UINT width, UINT height, std::wstring name) :
	m_width(width),
	m_height(height),
	m_title(name),
	m_useWarpDevice(false)
{
	WCHAR assetPath[512];
	GetAssetsPath(assetPath,_countof(assetPath));
	m_assetPath = assetPath;

	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);

	if (fopen_s(&m_file, "Log.txt", "w") == 0)
	{
		fclose(m_file);
		LogToFile("Log File Opened\n");
	}
	else
	{
		MessageBox(NULL,TEXT("Log File Could Not Be Opened\n"),TEXT("Error"),MB_ICONERROR);
	}
}

GraphicsBase::~GraphicsBase()
{

}

void GraphicsBase::UpdateSize(UINT width, UINT height)
{
	m_width = width;
	m_height = height;
	m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
}


std::wstring GraphicsBase::GetAssetFullPath(LPCWSTR assetName)
{
	return m_assetPath + assetName;
}


//IDXGIFactory is used to create swap chain
void GraphicsBase::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	//Enumerate Adapters
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		//Check for available D3D12 device but do not create
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}

void GraphicsBase::SetCustomWindowText(LPCWSTR text)
{
	std::wstring windowText = m_title + L": " + text;
	SetWindowText(BaseWindowClass::GetHwnd(),windowText.c_str());
}

void GraphicsBase::LogToFile(const char *message)
{
	if (fopen_s(&m_file, "Log.txt", "a") == 0)
	{
		fprintf_s(m_file, message);
		fclose(m_file);
	}
}
