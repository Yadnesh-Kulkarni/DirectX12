#include "stdafx.h"
#include "D3D12HelloWorld.h"

#pragma comment(lib, "d3dcompiler.lib")

D3D12HelloTriangle::D3D12HelloTriangle(UINT width, UINT height, std::wstring name) :
	GraphicsBase(width, height, name),
	m_frameIndex(0),
	m_rtvDescriptorSize(0),
	m_viewport(0.0f,0.0f, static_cast<float>(width),static_cast<float>(height)),
	m_scissorRect(0,0,static_cast<LONG>(width),static_cast<LONG>(height))
{	

}

void D3D12HelloTriangle::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

void D3D12HelloTriangle::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	//Create A DXGI Factory which is a generic interface for procuring Tech independant resources
	//We are going to use this interface for obtaining Device and SwapChain
	ComPtr<IDXGIFactory4> factory;
	if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory))))
	{
		LogToFile("Creating Direct X Graphics Interface Factory Failed\n");
		return;
	}


	//Create Hardware Adapter by all available enumerating adapters	
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	GetHardwareAdapter(factory.Get(),&hardwareAdapter);
	
	//Create Hardware Device From Hardware Adapter which supports Feature Level 12
	if (FAILED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device))))
	{
		LogToFile("Unable to obtain hardware adapter\n");
	}
	
	//Create a command queue
	{
		//Command Queue is a structure from which GPU executes all the commands

		//Create Command Queue Descriptor which details properties of Command Queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		//Create Command Queue
		if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue))))
		{
			LogToFile("Failed To Create Command Queue\n");
			return;
		}
	}

	//Create Swap Chain
	{
		//Swap chain consists of buffers which are swapped to create animation

		//Create Swap Chain Descriptor which details properties of Swap Chain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = FrameCount;
		swapChainDesc.Width = m_width;
		swapChainDesc.Height = m_height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		//Create Swap Chain
		ComPtr<IDXGISwapChain1> swapChain;
		if (FAILED(factory->CreateSwapChainForHwnd(m_commandQueue.Get(), BaseWindowClass::GetHwnd(), &swapChainDesc, nullptr, nullptr, &swapChain)))
		{
			LogToFile("Unable to create swap chain\n");
			return;
		}

		//Assign local swap chain to class variable
		swapChain.As(&m_swapChain);

		//Save the frame index current back buffer from swap chain
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}


	//Create Descriptor Heaps
	//Descriptor heaps are descriptor arrays
	{
		//Create Descriptor heap for Render Target View which is target for drawing
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount; //One each for swap chain buffers
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//Specifying type as Render Target View
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		//Create Descriptor Heaps using device
		if (FAILED(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap))))
		{
			LogToFile("Creation of descriptor heap failed\n");
		}

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	//This function creates render target views for all swap chain buffers
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
		m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);

		NAME_D3D12_OBJECT_INDEXED(m_renderTargets, n);
	}

	//Create Command Allocator
	//Command Allocator is a structure that actually holds command list
	if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator))))
	{
		LogToFile("Failed To Create Command Allocator");
		return;
	}
}

void D3D12HelloTriangle::LoadAssets()
{
	//Create Root Signature Descriptor

	//Fill up root signature descriptor
	D3D12_ROOT_SIGNATURE_DESC rootSignDesc;
	rootSignDesc.NumParameters = 0;
	rootSignDesc.pParameters = nullptr;
	rootSignDesc.NumStaticSamplers = 0;
	rootSignDesc.pStaticSamplers = nullptr;
	rootSignDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	//Serialize the Root Signature Descriptor
	if (FAILED(D3D12SerializeRootSignature(&rootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error)))
	{
		LogToFile("D3D12SerializeRootSignature Failed\n");
		return;
	}

	//Create Root Signature
	if (FAILED(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature))))
	{
		LogToFile("Create Root Signature Failed\n");
		return;
	}


	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;
	
#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	if (FAILED(D3DCompileFromFile(
		L"VertexShader.hlsl", //Shader String
		nullptr,// Defines
		nullptr,// Includes
		"VSMain",// Entry Point
		"vs_5_0",// Shader Version
		compileFlags,// compile flags
		0,
		&vertexShader,
		nullptr)))
	{
		LogToFile("Unable to compile vertex shader\n");
		return;
	}

	if (FAILED(D3DCompileFromFile(
		L"PixelShader.hlsl",
		nullptr,
		nullptr,
		"PSMain",
		"ps_5_0",
		compileFlags,
		0,
		&pixelShader,
		nullptr)))
	{
		LogToFile("Unable to compile pixel shader\n");
		return;
	}

	//Create Input Element Descriptors which describe input element classes
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2];

	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[0].InputSlot = 0;
	inputElementDescs[0].AlignedByteOffset = 0;
	inputElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescs[0].InstanceDataStepRate = 0;

	inputElementDescs[1].SemanticName = "COLOR";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[1].InputSlot = 0;
	inputElementDescs[1].AlignedByteOffset = 12;
	inputElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescs[1].InstanceDataStepRate = 0;

	/*D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};*/
	

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_RootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	if (FAILED(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState))))
	{
		LogToFile("Failed to create graphics pipeline state\n");
		return;
	}


	//Create Command List
	if (FAILED(m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(),
		m_pipelineState.Get(),
		IID_PPV_ARGS(&m_commandList))))
	{
		LogToFile("Failed to create command list\n");
		return;
	}

	//Command list is always created in record state that is why we close it immediately
	if (FAILED(m_commandList->Close()))
	{
		LogToFile("Unable to close command list\n");
		return;
	}

	Vertex triangleData[] =
	{
			{ { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	const UINT vertexBufferSize = sizeof(triangleData);

	if (FAILED(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer))))
	{
		LogToFile("Failed to Create Committed Resource\n");
		return;
	}

	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	if (FAILED(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin))))
	{
		LogToFile("Mapping vertex buffer failed");
		return;
	}
	memcpy(pVertexDataBegin, triangleData, sizeof(triangleData));
	m_vertexBuffer->Unmap(0, nullptr);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = vertexBufferSize;

	//Create Synchronization Objects to make sure resources are uploaded to GPU memory

	if (FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
	{
		LogToFile("Unable to create fence\n");
		return;
	}
	m_fenceValue = 1;

	m_fenceEvent = CreateEvent(nullptr,FALSE,FALSE,nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	WaitForPreviousFrame();
}

void D3D12HelloTriangle::OnUpdate()
{

}

void D3D12HelloTriangle::OnRender()
{
	PopulateCommandList();

	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	
	//Present The Frame
	if (FAILED(m_swapChain->Present(1, 0)))
	{
		LogToFile("Swap Chain Present Failed\n");
	}

	WaitForPreviousFrame();
}

void D3D12HelloTriangle::OnDestroy()
{
	WaitForPreviousFrame();

	CloseHandle(m_fenceEvent);
}

void D3D12HelloTriangle::OnSizeChanged(UINT width, UINT height)
{
	if (width != m_width || height != m_height)
	{
		LogToFile("In Size Changed 1\n");
		WaitForPreviousFrame();

		for (UINT n = 0; n < FrameCount; n++)
		{
			m_renderTargets[n].Reset();
			//m_fenceValues[n] - m_fenceValue[m_frameIndex];
		}

		LogToFile("In Size Changed 2\n");
		DXGI_SWAP_CHAIN_DESC desc = {};
		m_swapChain->GetDesc(&desc);
		if (FAILED(m_swapChain->ResizeBuffers(FrameCount, width, height, desc.BufferDesc.Format, desc.Flags)))
		{
			LogToFile("Swap Chain Resize Failed\n");
		}
		
		LogToFile("In Size Changed 3\n");
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		UpdateSize(width, height);

		LoadSizeDependentResources();
	}
}

void D3D12HelloTriangle::PopulateCommandList()
{
	//Command List Allocators can only be reset when the associated command
	// lists have finished execution
	//Fences should be used to determing GPU execution progress
	ThrowIfFailed(m_commandAllocator->Reset());

	//Command list can be reset at any time after ExecuteCommandList has been called
	//Command list reset must be done before re-recording command list
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));
	
	m_commandList->SetGraphicsRootSignature(m_RootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	//Record Commands
	const float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0,1,&m_vertexBufferView);
	m_commandList->DrawInstanced(3, 1, 0, 0);


	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_commandList->Close());
}

void D3D12HelloTriangle::WaitForPreviousFrame()
{
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(),fence));
	m_fenceValue++;

	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void D3D12HelloTriangle::LoadSizeDependentResources()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
		m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);

		NAME_D3D12_OBJECT_INDEXED(m_renderTargets, n);
	}
}

void D3D12HelloTriangle::OnKeyDown(UINT key)
{
	switch (key)
	{
	case 0x46:
	{
		BOOL fullscreenstate;
		ThrowIfFailed(m_swapChain->GetFullscreenState(&fullscreenstate, nullptr));
		if (FAILED(m_swapChain->SetFullscreenState(!fullscreenstate, nullptr)))
		{
			OutputDebugString(TEXT("Setting Fullscreen Failed!"));
		}
	}
		break;
	}
}