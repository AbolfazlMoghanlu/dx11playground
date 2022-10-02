#pragma once

#include "pch.h"
#include "Window/Window.h"
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Imgui/imgui.h"
#include "imgui/backends/imgui_impl_dx12.h"
#include "imgui/backends/imgui_impl_win32.h"

using namespace Microsoft::WRL;

const int WindowWidth = 1080;
const int WindowHeight = 720;

Rotatorf CameraRotation = Rotatorf(0.0f);
Vector3f CameraPosition = Vector3f(0.0f);
const float MouseSpeed = 500.0f;
const float CameraSpeed = 10.0f;

Rotatorf LightDirection = Rotatorf(90.0f, 0.0f, 0.0f);

Window* MainWindow;


struct Vertex
{
	Vector3f position;
	Vector3f color;
	float U;
};

struct VSConstantBufferLayout
{
	Matrix<float> TransformMatrix;
	Matrix<float> ViewMatrix;
	Matrix<float> ProjectionMatrix;
	float padding[16] = { 0 };
};

VSConstantBufferLayout VSConstantBuffer;

const int FrameCount = 2;

D3D12_VIEWPORT m_viewport;
D3D12_RECT m_scissorRect;
ComPtr<IDXGISwapChain3> m_swapChain;
ComPtr<ID3D12Device> m_device;
ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
ComPtr<ID3D12CommandAllocator> m_commandAllocator;
ComPtr<ID3D12CommandQueue> m_commandQueue;
ComPtr<ID3D12RootSignature> m_rootSignature;
ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
ComPtr<ID3D12DescriptorHeap> m_ConstantHeap;
ComPtr<ID3D12DescriptorHeap> m_ImGuiHeap;
ComPtr<ID3D12PipelineState> m_pipelineState;
ComPtr<ID3D12GraphicsCommandList> m_commandList;
UINT m_rtvDescriptorSize;
ComPtr<ID3D12Resource> m_constantBuffer;
UINT8* m_pCbvDataBegin;
ComPtr<ID3D12Resource> m_cloudBuffer;
UINT8* m_pCloudbvDataBegin;

ComPtr<ID3D12Resource> m_vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
ComPtr<ID3D12Resource> m_CoverageTexture;
ComPtr<ID3D12Resource> textureUploadHeap;

ComPtr<ID3D12Resource> m_WorleyBaseTexture;
ComPtr<ID3D12Resource> m_WorleyBaseUpload;

ComPtr<ID3D12Resource> m_WorleyDetailTexture;
ComPtr<ID3D12Resource> m_WorleyDetailUpload;


UINT m_frameIndex;
HANDLE m_fenceEvent;
ComPtr<ID3D12Fence> m_fence;
UINT64 m_fenceValue;


struct PCloudBufferLayout
{
	Vector3f CameraPosition;
	float UselessData;

	Vector3f CloudColor = Vector3f(1.0f, 1.0f, 1.0f);
	float StartZ = 5000.0f;

	int Steps = 64;
	float Height = 10000.0f;
	float Coverage = 0.8f;
	float CoveragemapScale = 100000.0f;

	float DensityScale = 0.2f;
	float BottomRoundness = 0.07f;
	float TopRoundness = 0.2f;
	float BottomDensity = 0.15f;

	float TopDensity = 0.9f;
	float BaseNoiseScale = 35000.0f;
	float DetailNoiseScale = 20000.0f;
	float Anvil = 0.5f;

	float TracingStartMaxDistance = 350000;
	float DetailNoiseIntensity = 0.8f;
	int LightSteps = 32;
	float LightStepSize = 100000.0f;

	Vector3f LightDir;
	float Useless3 = 0.0f;

	float Padding[36];
};

PCloudBufferLayout PsCloudBL;

void WaitForPreviousFrame()
{
	const UINT64 fence = m_fenceValue;
	m_commandQueue->Signal(m_fence.Get(), fence);
	m_fenceValue++;

	if (m_fence->GetCompletedValue() < fence)
	{
		m_fence->SetEventOnCompletion(fence, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void SetViewportSize(int X, int Y)
{
	m_viewport.Width = float(X);
	m_viewport.Height = float(Y);
	m_viewport.MinDepth = 0;
	m_viewport.MaxDepth = 1;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	m_scissorRect.left = 0;
	m_scissorRect.right = X;
	m_scissorRect.top = 0;
	m_scissorRect.bottom = Y;
}

void LoadPipeline()
{
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}

	ComPtr<IDXGIFactory4> factory;
	CreateDXGIFactory1(IID_PPV_ARGS(&factory));

	ComPtr<IDXGIAdapter> Adapter;
	//	factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));


	SIZE_T maxDedicatedVideoMemory = 0;
	UINT Index = 0;
	for (UINT i = 0; factory->EnumAdapters(i, &Adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC AdaptorDesc;
		Adapter->GetDesc(&AdaptorDesc);

		if (AdaptorDesc.DedicatedVideoMemory > maxDedicatedVideoMemory)
		{
			maxDedicatedVideoMemory = AdaptorDesc.DedicatedVideoMemory;
			Index = i;
		}
	}

	factory->EnumAdapters(Index, &Adapter);

	D3D12CreateDevice(
		Adapter.Get(),
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_device)
	);





	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.BufferDesc.Width = WindowWidth;
	swapChainDesc.BufferDesc.Height = WindowHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = MainWindow->GetHandle();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = true;

	ComPtr<IDXGISwapChain> swapChain;
	HRESULT a = factory->CreateSwapChain(
		m_commandQueue.Get(),
		&swapChainDesc,
		&swapChain
	);

	swapChain.As(&m_swapChain);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = 2;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}


	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = 10;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_ConstantHeap));


	m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
}

void LoadAssets()
{
	D3D12_DESCRIPTOR_RANGE ranges[5];
	D3D12_ROOT_PARAMETER rootParameters[1];

	ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	ranges[0].NumDescriptors = 1;
	ranges[0].BaseShaderRegister = 0;
	ranges[0].OffsetInDescriptorsFromTableStart = 0;
	ranges[0].RegisterSpace = 0;

	ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	ranges[1].NumDescriptors = 1;
	ranges[1].BaseShaderRegister = 0;
	ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	ranges[1].RegisterSpace = 0;

	ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	ranges[2].NumDescriptors = 1;
	ranges[2].BaseShaderRegister = 1;
	ranges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	ranges[2].RegisterSpace = 0;

	ranges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	ranges[3].NumDescriptors = 1;
	ranges[3].BaseShaderRegister = 2;
	ranges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	ranges[3].RegisterSpace = 0;

	ranges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	ranges[4].NumDescriptors = 1;
	ranges[4].BaseShaderRegister = 1;
	ranges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	ranges[4].RegisterSpace = 0;

	D3D12_ROOT_DESCRIPTOR_TABLE DescriptorTable;
	DescriptorTable.NumDescriptorRanges = _countof(ranges);
	DescriptorTable.pDescriptorRanges = &ranges[0];

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].DescriptorTable = DescriptorTable;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC StaticSamplerDesc = {};
	StaticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	StaticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	StaticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	StaticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	StaticSamplerDesc.MaxAnisotropy = 16;
	StaticSamplerDesc.MaxLOD = 1;
	StaticSamplerDesc.MinLOD = 0;
	StaticSamplerDesc.MipLODBias = 0;
	StaticSamplerDesc.RegisterSpace = 0;
	StaticSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	StaticSamplerDesc.ShaderRegister = 0;
	StaticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC StaticSamplers[] = { StaticSamplerDesc };

	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(_countof(rootParameters), rootParameters, _countof(StaticSamplers), &StaticSamplers[0]
			, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

		//std::cout << reinterpret_cast<const char*>(error->GetBufferPointer()) << std::flush;

		m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
	}

	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

		D3DCompileFromFile(L"Source/Shader/VertexShader_vs.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
		D3DCompileFromFile(L"Source/Shader/SkyPixelShader_ps.hlsl", nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "U", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_RENDER_TARGET_BLEND_DESC TargetBlendDesc;
		TargetBlendDesc.BlendEnable = true;
		TargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		TargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		TargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		TargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		TargetBlendDesc.DestBlendAlpha = D3D12_BLEND_DEST_ALPHA;
		TargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		TargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		TargetBlendDesc.LogicOpEnable = FALSE;

		D3D12_BLEND_DESC BlendDesc;
		BlendDesc.AlphaToCoverageEnable = false;
		BlendDesc.IndependentBlendEnable = false;
		BlendDesc.RenderTarget[0] = TargetBlendDesc;



		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
		psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = BlendDesc;
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
	}


	m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList));

	{
		Vertex triangleVertices[] =
		{
			{ { -0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, 0.0f },
			{ { 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 0.0f },
			{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, 0.0f },
			{ { -0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, 0.0f },
			{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, 0.0f },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 0.0f }
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		m_device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer));


		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		m_vertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
	}

	// -----------------------------------------------------------

	const UINT ConstantBufferSize = sizeof(VSConstantBufferLayout);
	m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(ConstantBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_constantBuffer));


	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = ConstantBufferSize;
	m_device->CreateConstantBufferView(&cbvDesc, m_ConstantHeap->GetCPUDescriptorHandleForHeapStart());


	CD3DX12_RANGE readRange(0, 0);
	m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin));
	memcpy(m_pCbvDataBegin, &VSConstantBuffer, sizeof(VSConstantBuffer));


	// -------------------------------------------------------------------

	unsigned char* CoverageTextureBlob;
	int CoverageImageWidth, CoverageImageHeight, CoverageImageNumberOfChannels;

	CoverageTextureBlob = stbi_load("../Content/T_WeatherNoiseBetter.png",
		&CoverageImageWidth, &CoverageImageHeight, &CoverageImageNumberOfChannels, 0);


	D3D12_RESOURCE_DESC CoverageTextureDesc = {};
	CoverageTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	CoverageTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	CoverageTextureDesc.Width = CoverageImageWidth;
	CoverageTextureDesc.Height = CoverageImageHeight;
	CoverageTextureDesc.DepthOrArraySize = 1;
	CoverageTextureDesc.MipLevels = 1;
	CoverageTextureDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	CoverageTextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	CoverageTextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	CoverageTextureDesc.SampleDesc.Count = 1;


	D3D12_HEAP_PROPERTIES CoverageTextureHeapProperties = {};
	CoverageTextureHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	CoverageTextureHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	CoverageTextureHeapProperties.CreationNodeMask = 0;
	CoverageTextureHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	CoverageTextureHeapProperties.VisibleNodeMask = 0;

	m_device->CreateCommittedResource(&CoverageTextureHeapProperties, D3D12_HEAP_FLAG_NONE, &CoverageTextureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_CoverageTexture));




	UINT64 uploadBufferSize;
	auto Desc = m_CoverageTexture->GetDesc();
	m_device->GetCopyableFootprints(&Desc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUploadHeap));



	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = CoverageTextureBlob;
	textureData.RowPitch = CoverageImageWidth * sizeof(unsigned char) * 4;
	textureData.SlicePitch = textureData.RowPitch * CoverageImageHeight;



	UpdateSubresources(m_commandList.Get(), m_CoverageTexture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CoverageTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = CoverageTextureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	UINT S = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE H(m_ConstantHeap->GetCPUDescriptorHandleForHeapStart());
	H.Offset(1, S);

	m_device->CreateShaderResourceView(m_CoverageTexture.Get(), &srvDesc, H);

	// -----------------------------------------------------------------------

	unsigned char* WorleyTextureBlob;
	int WorleyImageWidth, WorleyImageHeight, WorleyImageNumberOfChannels;

	WorleyTextureBlob = stbi_load("../Content/T_WorleyNoise.png",
		&WorleyImageWidth, &WorleyImageHeight, &WorleyImageNumberOfChannels, 0);

	D3D12_RESOURCE_DESC WorleyBaseTextureDesc = {};
	WorleyBaseTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	WorleyBaseTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	WorleyBaseTextureDesc.Width = 128;
	WorleyBaseTextureDesc.Height = 128;
	WorleyBaseTextureDesc.DepthOrArraySize = 128;
	WorleyBaseTextureDesc.DepthOrArraySize = 1;
	WorleyBaseTextureDesc.MipLevels = 1;
	WorleyBaseTextureDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	WorleyBaseTextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	WorleyBaseTextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	WorleyBaseTextureDesc.SampleDesc.Count = 1;


	D3D12_HEAP_PROPERTIES WorleyBaseDestProperties = {};
	WorleyBaseDestProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	WorleyBaseDestProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	WorleyBaseDestProperties.CreationNodeMask = 0;
	WorleyBaseDestProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	WorleyBaseDestProperties.VisibleNodeMask = 0;

	m_device->CreateCommittedResource(&WorleyBaseDestProperties, D3D12_HEAP_FLAG_NONE, &WorleyBaseTextureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_WorleyBaseTexture));


	Desc = m_WorleyBaseTexture->GetDesc();
	m_device->GetCopyableFootprints(&Desc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_WorleyBaseUpload));


	D3D12_SUBRESOURCE_DATA WorleyBasetextureData = {};
	WorleyBasetextureData.pData = WorleyTextureBlob;
	WorleyBasetextureData.RowPitch = sizeof(char) * WorleyImageNumberOfChannels * 128;
	WorleyBasetextureData.SlicePitch = sizeof(char) * WorleyImageNumberOfChannels * 128 * 128;


	UpdateSubresources(m_commandList.Get(), m_WorleyBaseTexture.Get(), m_WorleyBaseUpload.Get(), 0, 0, 1, &WorleyBasetextureData);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_WorleyBaseTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));


	D3D12_SHADER_RESOURCE_VIEW_DESC srvWorleyBaseDesc = {};
	srvWorleyBaseDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvWorleyBaseDesc.Format = WorleyBaseTextureDesc.Format;
	srvWorleyBaseDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvWorleyBaseDesc.Texture3D.MipLevels = 1;
	srvWorleyBaseDesc.Texture3D.MostDetailedMip = 0;
	srvWorleyBaseDesc.Texture3D.ResourceMinLODClamp = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE H1(m_ConstantHeap->GetCPUDescriptorHandleForHeapStart());
	H1.Offset(2, S);

	m_device->CreateShaderResourceView(m_WorleyBaseTexture.Get(), &srvWorleyBaseDesc, H1);

	// -------------------------------------------------------------

	unsigned char* WorleyDetailTextureBlob;
	int WorleyDetailImageWidth, WorleyDetailImageHeight, WorleyDetailImageNumberOfChannels;

	WorleyDetailTextureBlob = stbi_load("../Content/T_WorleyNoise_Detail.png",
		&WorleyDetailImageWidth, &WorleyDetailImageHeight, &WorleyDetailImageNumberOfChannels, 0);

	D3D12_RESOURCE_DESC WorleyDetailTextureDesc = {};
	WorleyDetailTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	WorleyDetailTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	WorleyDetailTextureDesc.Width = 32;
	WorleyDetailTextureDesc.Height = 32;
	WorleyDetailTextureDesc.DepthOrArraySize = 32;
	WorleyDetailTextureDesc.DepthOrArraySize = 1;
	WorleyDetailTextureDesc.MipLevels = 1;
	WorleyDetailTextureDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	WorleyDetailTextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	WorleyDetailTextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	WorleyDetailTextureDesc.SampleDesc.Count = 1;


	D3D12_HEAP_PROPERTIES WorleyDetailDestProperties = {};
	WorleyDetailDestProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	WorleyDetailDestProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	WorleyDetailDestProperties.CreationNodeMask = 0;
	WorleyDetailDestProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	WorleyDetailDestProperties.VisibleNodeMask = 0;

	m_device->CreateCommittedResource(&WorleyDetailDestProperties, D3D12_HEAP_FLAG_NONE, &WorleyDetailTextureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_WorleyDetailTexture));


	Desc = m_WorleyDetailTexture->GetDesc();
	m_device->GetCopyableFootprints(&Desc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_WorleyDetailUpload));


	D3D12_SUBRESOURCE_DATA WorleyDetailtextureData = {};
	WorleyDetailtextureData.pData = WorleyDetailTextureBlob;
	WorleyDetailtextureData.RowPitch = sizeof(char) * WorleyDetailImageNumberOfChannels * 32;
	WorleyDetailtextureData.SlicePitch = sizeof(char) * WorleyDetailImageNumberOfChannels * 32 * 32;


	UpdateSubresources(m_commandList.Get(), m_WorleyDetailTexture.Get(), m_WorleyDetailUpload.Get(), 0, 0, 1, &WorleyDetailtextureData);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_WorleyDetailTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));


	D3D12_SHADER_RESOURCE_VIEW_DESC srvWorleyDetailDesc = {};
	srvWorleyDetailDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvWorleyDetailDesc.Format = WorleyDetailTextureDesc.Format;
	srvWorleyDetailDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvWorleyDetailDesc.Texture3D.MipLevels = 1;
	srvWorleyDetailDesc.Texture3D.MostDetailedMip = 0;
	srvWorleyDetailDesc.Texture3D.ResourceMinLODClamp = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE H2(m_ConstantHeap->GetCPUDescriptorHandleForHeapStart());
	H2.Offset(3, S);

	m_device->CreateShaderResourceView(m_WorleyDetailTexture.Get(), &srvWorleyDetailDesc, H2);

	// -----------------------------------------------------------

	const UINT ConstantBufferSize1 = sizeof(PCloudBufferLayout);
	m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(ConstantBufferSize1), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_cloudBuffer));


	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc1 = {};
	cbvDesc1.BufferLocation = m_cloudBuffer->GetGPUVirtualAddress();
	cbvDesc1.SizeInBytes = ConstantBufferSize1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE H5(m_ConstantHeap->GetCPUDescriptorHandleForHeapStart());
	H5.Offset(4, S);
	m_device->CreateConstantBufferView(&cbvDesc1, H5);

	CD3DX12_RANGE readRange1(0, 0);
	m_cloudBuffer->Map(0, &readRange1, reinterpret_cast<void**>(&m_pCloudbvDataBegin));
	memcpy(m_pCloudbvDataBegin, &PsCloudBL, sizeof(PsCloudBL));

	// -------------------------------------------------

	m_commandList->Close();

	{
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	{
		m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
		m_fenceValue = 1;

		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			HRESULT_FROM_WIN32(GetLastError());
		}

		WaitForPreviousFrame();
	}
}


void PopulateCommandList()
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	ID3D12DescriptorHeap* ppHeaps[] = { m_ConstantHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_ConstantHeap->GetGPUDescriptorHandleForHeapStart());

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);


	// Record commands.
	const float clearColor[] = { 0.47f, 0.78f, 0.89f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(6, 1, 0, 0);





	ppHeaps[0] = m_ImGuiHeap.Get();
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());

	// Indicate that the back buffer will now be used to present.
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &barrier);


	m_commandList->Close();
	//WaitForPreviousFrame();
}


void OnRender()
{
	PopulateCommandList();

	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_swapChain->Present(0, 0);

	WaitForPreviousFrame();
}



int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, int nCmdShow)
{
	Window::Startup();

	MainWindow = new Window(L"DxWindow", WindowWidth, WindowHeight);

	AllocConsole();

	static std::ofstream conout("CONOUT$", std::ios::out);
	// Set std::cout stream buffer to conout's buffer (aka redirect/fdreopen)
	std::cout.rdbuf(conout.rdbuf());

	MainWindow->Tick(1);

	LoadPipeline();
	LoadAssets();


	D3D12_DESCRIPTOR_HEAP_DESC ImGuiHeapDesc = {};
	ImGuiHeapDesc.NumDescriptors = 3;
	ImGuiHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ImGuiHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_device->CreateDescriptorHeap(&ImGuiHeapDesc, IID_PPV_ARGS(&m_ImGuiHeap));


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui::StyleColorsDark();

	m_ImGuiHeap->SetName(L"GuiHeap");

	ImGui_ImplWin32_Init(MainWindow->GetHandle());
	ImGui_ImplDX12_Init(m_device.Get(), FrameCount, DXGI_FORMAT_R8G8B8A8_UNORM, m_ImGuiHeap.Get(),
		m_ImGuiHeap->GetCPUDescriptorHandleForHeapStart(), m_ImGuiHeap->GetGPUDescriptorHandleForHeapStart());

	bool show_demo_window = true;


	while (MainWindow->IsOpen())
	{
		if (MainWindow->IsRightClickDown())
		{
			float CameraPitchOffset = MouseSpeed * -MainWindow->MouseDeltaY;
			float CameraYawOffset = MouseSpeed * MainWindow->MouseDeltaX;

			Rotatorf DeltaRotation = Rotatorf(CameraPitchOffset, CameraYawOffset, 0.0f);
			CameraRotation = Rotatorf::CombineRotators(CameraRotation, DeltaRotation);

			float CameraPitchClamped = CameraRotation.Pitch < 180.0f ?
				Math::Clamp<float>(CameraRotation.Pitch, 1.0f, 89.0f) : Math::Clamp<float>(CameraRotation.Pitch, 271.0f, 359.0f);

			CameraRotation = Rotatorf(CameraPitchClamped, CameraRotation.Yaw, CameraRotation.Roll);
		}
		std::cout << CameraRotation.ToString() << std::endl;

		Vector3f ForwardVector = CameraRotation.Vector();
		Vector3f RightVector = Vector3f::CrossProduct(Vector3f::UpVector, ForwardVector);

		Vector3f CameraForwardOffset = ForwardVector * MainWindow->GetUpValue() * CameraSpeed;
		Vector3f CameraRightOffset = RightVector * MainWindow->GetRightValue() * CameraSpeed;

		//CameraPosition = CameraPosition + CameraForwardOffset + CameraRightOffset;
		std::cout << CameraPosition.ToString() << std::endl;

		// ---------------------------------------------------------

		auto Time = std::chrono::high_resolution_clock();
		auto Now = Time.now();

		float TimeSeconds = (float)std::chrono::duration_cast<std::chrono::seconds>(Now.time_since_epoch()).count();

		MainWindow->Tick(1);

		{
			Vector3f CameraForwardVector = CameraRotation.Vector();

			Vector3f SkyPosition = CameraPosition + CameraForwardVector * 1.0f;
			Rotatorf SkyRotation = CameraRotation;
			Vector3f SkyScale = Vector3f(5.0f);

			Matrix<float> TransformMatrix = ScaleRotationTranslationMatrix<float>(SkyScale, SkyRotation, SkyPosition);
			VSConstantBuffer.TransformMatrix = TransformMatrix;

			Matrix<float> CameraViewMatrix = Math::LookAt(CameraPosition, CameraForwardVector, Vector3f::UpVector);
			VSConstantBuffer.ViewMatrix = CameraViewMatrix;

			Matrix<float> ProjectionMatrix = PerspectiveMatrix<float>(90.0f, (float)WindowWidth / (float)WindowHeight, 0.1f, 100.0f);
			VSConstantBuffer.ProjectionMatrix = ProjectionMatrix;

			// ---------------------------------------------------------

			memcpy(m_pCbvDataBegin, &VSConstantBuffer, sizeof(VSConstantBuffer));
			memcpy(m_pCloudbvDataBegin, &PsCloudBL, sizeof(PsCloudBL));
		}

		SetViewportSize(WindowWidth, WindowHeight);




		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		bool show_another_window = true;
		ImGui::Begin("Setting", &show_another_window, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);


		ImGui::ColorEdit3("CloudColor", &PsCloudBL.CloudColor.X);
		ImGui::SliderInt("Steps", &PsCloudBL.Steps, 2, 256);
		ImGui::SliderFloat("Coverage", &PsCloudBL.Coverage, 0.0f, 2.0f, "%1f");
		ImGui::SliderFloat("CoverageScale", &PsCloudBL.CoveragemapScale, 50000.0f, 500000.0f, "%1f");
		ImGui::SliderFloat("StartZ", &PsCloudBL.StartZ, 1.0f, 20000.0f, "%1f");
		ImGui::SliderFloat("Height", &PsCloudBL.Height, 1.0f, 20000.0f, "%1f");
		ImGui::SliderFloat("DensityScale", &PsCloudBL.DensityScale, 0.0f, 2.0f, "%1f");
		ImGui::SliderFloat("BottomRoundness", &PsCloudBL.BottomRoundness, 0.0f, 1.0f, "%1f");
		ImGui::SliderFloat("TopRoundness", &PsCloudBL.TopRoundness, 0.0f, 1.0f, "%1f");
		ImGui::SliderFloat("BottomDensity", &PsCloudBL.BottomDensity, 0.0f, 1.0f, "%1f");
		ImGui::SliderFloat("TopDensity", &PsCloudBL.TopDensity, 0.0f, 1.0f, "%1f");
		ImGui::SliderFloat("BaseNoiseScale", &PsCloudBL.BaseNoiseScale, 5000.0f, 50000.0f, "%1f");
		ImGui::SliderFloat("DetailNoiseScale", &PsCloudBL.DetailNoiseScale, 500.0f, 30000.0f, "%1f");
		ImGui::SliderFloat("DetailNoiseIntensity", &PsCloudBL.DetailNoiseIntensity, 0.0f, 1.0f, "%1f");
		ImGui::SliderFloat("Anvil", &PsCloudBL.Anvil, 0.0f, 1.0f, "%1f");
		ImGui::SliderFloat("TracingStartMaxDistance", &PsCloudBL.TracingStartMaxDistance, 100000.0, 500000.0, "%1f");
		ImGui::SliderInt("LightSteps", &PsCloudBL.LightSteps, 1, 32);
		ImGui::SliderFloat("LightStepSize", &PsCloudBL.LightStepSize, 1.0, 1000000.0, "%1f");
		ImGui::SliderFloat3("LightDirection", &LightDirection.Pitch, 0.0, 360.0, "%1f");

		PsCloudBL.LightDir = LightDirection.Vector();
		CameraPosition = CameraPosition + CameraForwardOffset + CameraRightOffset;
		PsCloudBL.CameraPosition = CameraPosition;


		ImGui::End();


		ImGui::EndFrame();


		OnRender();
	}


	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Window::ShutDown();

	return 0;
}
