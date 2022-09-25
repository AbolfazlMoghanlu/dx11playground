#pragma once

#include "pch.h"
#include "Window/Window.h"
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Imgui/imgui.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include "imgui/backends/imgui_impl_win32.h"

const int WindowWidth = 1080; 
const int WindowHeight = 720; 

Rotatorf CameraRotation = Rotatorf(0.0f);
const float MouseSpeed = 500.0f;
const float CameraSpeed = 1.0f;

Vector3f CameraPosition = Vector3f(0.0f);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, int nCmdShow)
{
	Window::Startup();

	Window MainWindow(L"DxWindow", WindowWidth, WindowHeight);


	AllocConsole();
	
	static std::ofstream conout("CONOUT$", std::ios::out);
	// Set std::cout stream buffer to conout's buffer (aka redirect/fdreopen)
	std::cout.rdbuf(conout.rdbuf());
	

	Microsoft::WRL::ComPtr<ID3D11Device> Device;
	Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferDesc.Width = WindowWidth;
	SwapChainDesc.BufferDesc.Height = WindowHeight;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.OutputWindow = MainWindow.GetHandle();
	SwapChainDesc.Windowed = TRUE;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc.Flags = 0;

	UINT CFlags = D3D11_CREATE_DEVICE_DEBUG;

	HRESULT Result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, CFlags, nullptr, 0, D3D11_SDK_VERSION,
		&SwapChainDesc, &SwapChain, &Device, nullptr, &DeviceContext);

	if (FAILED(Result))
	{
		return -1;
	}
	
	// -----------------------------------------------

	Microsoft::WRL::ComPtr<ID3D11Resource> BackBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> BackBufferView;

	SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(BackBuffer.GetAddressOf()));
	Device->CreateRenderTargetView(BackBuffer.Get(), nullptr, &BackBufferView);
	float ClearColor[] = { 0.47f, 0.78f, 0.89f, 1.0f };

	// ------------------------------------------------

	struct Vertex
	{
		float X;
		float Y;
		float Z;

		float R;
		float G;
		float B;
	};

// 	const Vertex SkyVertecies[] = 
// 	{
// 		{0.5f, 0.0f, 0.5f,		1.0f, 1.0f, 1.0f},
// 		{0.5f, 0.0f, -0.5f,		1.0f, 0.0f, 0.0f},
// 		{-0.5f, 0.0f, 0.5f,		0.0f, 1.0f, 0.0f},
// 		{-0.5f, 0.0f, -0.5f,	0.0f, 0.0f, 1.0f}
// 	};
// 
// 	const unsigned short int SkyIndices[] =
// 	{
// 		2, 1, 0,
// 		2, 3, 1
// 	};

	const Vertex SkyVertecies[] = 
	{
		{0.5f, 0.5f, 0.0f,		1.0f, 1.0f, 1.0f},
		{0.5f, -0.5f, 0.0f,		1.0f, 0.0f, 0.0f},
		{-0.5f, 0.5f, 0.0f,		0.0f, 1.0f, 0.0f},
		{-0.5f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f}
	};

	const unsigned short int SkyIndices[] =
	{
		2, 0, 1,
		2, 1, 3
	};


	const Vertex CubeVertecies[] = 
	{
		 {0.5 , 0.5  ,-0.5 ,		1.0f, 1.0f, 1.0f},	//0
		 {0.5 , -0.5 ,-0.5 ,		1.0f, 0.0f, 0.0f},	//1
		 {0.5 , 0.5  ,0.5  ,		0.0f, 1.0f, 0.0f},	//2
		 {0.5 , -0.5 ,0.5  ,		0.0f, 0.0f, 1.0f},	//3
		 {-0.5,  0.5 ,-0.5 ,		1.0f, 1.0f, 1.0f},	//4
		 {-0.5,  -0.5, -0.5,		1.0f, 0.0f, 0.0f},	//5
		 {-0.5,  0.5 ,0.5  ,		0.0f, 1.0f, 0.0f},	//6
		 {-0.5,  -0.5, 0.5 ,		0.0f, 0.0f, 1.0f}	//7
	};

	const unsigned short int CubeIndices[] =
	{
		4, 0, 1, 4, 1, 5, // front
		6, 2, 0, 6, 0, 4, // top
		0, 2, 3, 0, 3, 1, // right
		6, 4, 5, 6, 5, 7, // left
		5, 1, 3, 5, 3, 7, // bottom
		6, 3, 2, 6, 7, 3  // back
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	D3D11_BUFFER_DESC VertexBufferDesc;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.CPUAccessFlags = false;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.ByteWidth = sizeof(CubeVertecies);
	VertexBufferDesc.StructureByteStride = sizeof(Vertex);

	Microsoft::WRL::ComPtr<ID3D11Buffer> SkyVertexBuffer;
	D3D11_BUFFER_DESC SkyVertexBufferDesc = VertexBufferDesc;
	SkyVertexBufferDesc.ByteWidth = sizeof(SkyVertecies);

	D3D11_SUBRESOURCE_DATA VertexDataDesc;
	VertexDataDesc.pSysMem = CubeVertecies;

	D3D11_SUBRESOURCE_DATA SkyVertexDataDesc;
	SkyVertexDataDesc.pSysMem = SkyVertecies;

	Device->CreateBuffer(&VertexBufferDesc, &VertexDataDesc, &VertexBuffer);
	Device->CreateBuffer(&SkyVertexBufferDesc, &SkyVertexDataDesc, &SkyVertexBuffer);

	// ------------------------------------------------

	Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> VertexShaderBlob;

	D3DReadFileToBlob(L"../Bin/Debug-windows-x86_64/playground/VertexShader_vs.cso", VertexShaderBlob.GetAddressOf());
	Device->CreateVertexShader(VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), nullptr, &VertexShader);

	DeviceContext->VSSetShader(VertexShader.Get(), 0, 0);

	// ------------------------------------------------

	Microsoft::WRL::ComPtr<ID3D11InputLayout> VertexInputLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> SkyVertexInputLayout;

	const D3D11_INPUT_ELEMENT_DESC VertexInputElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	Device->CreateInputLayout(VertexInputElementDesc, (UINT)std::size(VertexInputElementDesc),
		VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), VertexInputLayout.GetAddressOf());
		
	Device->CreateInputLayout(VertexInputElementDesc, (UINT)std::size(VertexInputElementDesc),
		VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), SkyVertexInputLayout.GetAddressOf());

	// ------------------------------------------------

	D3D11_BUFFER_DESC IndexBufferDesc;
	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.ByteWidth = sizeof(CubeIndices);
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = false;
	IndexBufferDesc.MiscFlags = 0;
	IndexBufferDesc.StructureByteStride = sizeof(unsigned short int);

	D3D11_BUFFER_DESC SkyIndexBufferDesc;
	SkyIndexBufferDesc = IndexBufferDesc;
	SkyIndexBufferDesc.ByteWidth = sizeof(SkyIndices);

	D3D11_SUBRESOURCE_DATA IndexData;
	IndexData.pSysMem = CubeIndices;

	D3D11_SUBRESOURCE_DATA SkyIndexData;
	SkyIndexData.pSysMem = SkyIndices;

	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
	Device->CreateBuffer(&IndexBufferDesc, &IndexData, &IndexBuffer);

	Microsoft::WRL::ComPtr<ID3D11Buffer> SkyIndexBuffer;
	Device->CreateBuffer(&SkyIndexBufferDesc, &SkyIndexData, &SkyIndexBuffer);


	// ------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob> PixelShaderBlob;

	D3DReadFileToBlob(L"../Bin/Debug-windows-x86_64/playground/PixelShader_ps.cso", &PixelShaderBlob);
	Device->CreatePixelShader(PixelShaderBlob->GetBufferPointer(), PixelShaderBlob->GetBufferSize(), nullptr, &PixelShader);

	// ------------------------------------------------

	Microsoft::WRL::ComPtr<ID3D11PixelShader> SkyPixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob> SkyPixelShaderBlob;

	D3DReadFileToBlob(L"../Bin/Debug-windows-x86_64/playground/SkyPixelShader_ps.cso", &SkyPixelShaderBlob);
	Device->CreatePixelShader(SkyPixelShaderBlob->GetBufferPointer(), SkyPixelShaderBlob->GetBufferSize(), nullptr, &SkyPixelShader);

	// ------------------------------------------------

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ------------------------------------------------

	D3D11_VIEWPORT ViewportDesc;
	ViewportDesc.Width = WindowWidth;
	ViewportDesc.Height = WindowHeight;
	ViewportDesc.MinDepth = 0.0f;
	ViewportDesc.MaxDepth = 1.0f;
	ViewportDesc.TopLeftX = 0.0f;
	ViewportDesc.TopLeftY = 0.0f;

	DeviceContext->RSSetViewports(1, &ViewportDesc);

	// -----------------------------------------------

	struct VSContantBufferLayout
	{
		Matrix<float> TransformMatrix;
		Matrix<float> ViewMatrix;
		Matrix<float> ProjectionMatrix;
	};

	D3D11_BUFFER_DESC VsConstantBufferDesc;
	VsConstantBufferDesc.ByteWidth = sizeof(VSContantBufferLayout);
	VsConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	VsConstantBufferDesc.CPUAccessFlags = false;
	VsConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VsConstantBufferDesc.MiscFlags = 0;
	VsConstantBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA VSConstantData;

	VSContantBufferLayout VsConstantBL;
	Microsoft::WRL::ComPtr<ID3D11Buffer> VsConstantBuffer;

	// -----------------------------------------------

	struct PSContantBufferLayout
	{
		Vector3f CameraPosition;
		float UselessData = 1.0f;
	};

	D3D11_BUFFER_DESC PsConstantBufferDesc;
	PsConstantBufferDesc.ByteWidth = sizeof(PSContantBufferLayout);
	PsConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	PsConstantBufferDesc.CPUAccessFlags = false;
	PsConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	PsConstantBufferDesc.MiscFlags = 0;
	PsConstantBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA PSConstantData;

	PSContantBufferLayout PsConstantBL;
	Microsoft::WRL::ComPtr<ID3D11Buffer> PsConstantBuffer;

	// -----------------------------------------------

	struct PCloudBufferLayout
	{
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
		float BaseNoiseScale = 20000.0f;
		float DetailNoiseScale = 5000.0f;
		float Anvil = 0.5f;

		float TracingStartMaxDistance = 350000;
		float DetailNoiseIntensity = 0.8f;
		float Useless2 = 0.0f;
		float Useless3 = 0.0f;
	};

	D3D11_BUFFER_DESC PsCloudBufferDesc;
	PsCloudBufferDesc.ByteWidth = sizeof(PCloudBufferLayout);
	PsCloudBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	PsCloudBufferDesc.CPUAccessFlags = false;
	PsCloudBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	PsCloudBufferDesc.MiscFlags = 0;
	PsCloudBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA PSCloudData;

	PCloudBufferLayout PsCloudBL;
	Microsoft::WRL::ComPtr<ID3D11Buffer> PsCloudBuffer;

	// -----------------------------------------------


	D3D11_TEXTURE2D_DESC DepthDesc;
	DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthDesc.Width = WindowWidth;
	DepthDesc.Height = WindowHeight;
	DepthDesc.MipLevels = 1;
	DepthDesc.CPUAccessFlags = false;
	DepthDesc.MiscFlags = 0;
	DepthDesc.ArraySize = 1;
	DepthDesc.SampleDesc.Count = 1;
	DepthDesc.SampleDesc.Quality = 0;
	DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthDesc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> DepthStencilTexture;
	Device->CreateTexture2D(&DepthDesc, NULL, &DepthStencilTexture);


	D3D11_DEPTH_STENCIL_VIEW_DESC DepthViewDesc;
	DepthViewDesc.Format = DepthDesc.Format;
	DepthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DepthViewDesc.Flags = 0;
	DepthViewDesc.Texture2D.MipSlice = 0;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;
	Device->CreateDepthStencilView(DepthStencilTexture.Get(), &DepthViewDesc, &DepthStencilView);

	DeviceContext->OMSetRenderTargets(1, BackBufferView.GetAddressOf(), DepthStencilView.Get());


	D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
	DepthStencilDesc.DepthEnable = true;
	DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DepthStencilDesc.StencilEnable = true;
	DepthStencilDesc.StencilWriteMask = 255;
	DepthStencilDesc.StencilReadMask = 255;
	DepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	DepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
	DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
	DepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	
	DepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	DepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthState;
	Device->CreateDepthStencilState(&DepthStencilDesc, &DepthState);

	DeviceContext->OMSetDepthStencilState(DepthState.Get(), 128);

	// -----------------------------------------------------------------------

	unsigned char* CoverageTextureBlob;
	int CoverageImageWidth, CoverageImageHeight, CoverageImageNumberOfChannels;

	CoverageTextureBlob = stbi_load("../Content/T_WeatherNoiseBetter.png",
		&CoverageImageWidth, &CoverageImageHeight, &CoverageImageNumberOfChannels, 0);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> CoverageTexture;
	
	D3D11_TEXTURE2D_DESC CoverageTextureDescription;
	CoverageTextureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	CoverageTextureDescription.Height = CoverageImageHeight;
	CoverageTextureDescription.Width = CoverageImageWidth;
	CoverageTextureDescription.ArraySize = 1;
	CoverageTextureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	CoverageTextureDescription.CPUAccessFlags = 0;
	CoverageTextureDescription.MiscFlags = 0;
	CoverageTextureDescription.SampleDesc.Count = 1;
	CoverageTextureDescription.SampleDesc.Quality = 0;
	CoverageTextureDescription.Usage = D3D11_USAGE_DEFAULT;
	CoverageTextureDescription.MipLevels = 1;

	D3D11_SUBRESOURCE_DATA CoverageTextureData;
	CoverageTextureData.pSysMem = CoverageTextureBlob;
	CoverageTextureData.SysMemPitch = CoverageImageNumberOfChannels * CoverageImageWidth;
	
	Device->CreateTexture2D(&CoverageTextureDescription, &CoverageTextureData, &CoverageTexture);

	// ------------------------------------------------------------------

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CoverageTextureResourceView;

	D3D11_SHADER_RESOURCE_VIEW_DESC CoverageTextureResourceViewDesc;
	CoverageTextureResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	CoverageTextureResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	CoverageTextureResourceViewDesc.Texture2D.MipLevels = 1;
	CoverageTextureResourceViewDesc.Texture2D.MostDetailedMip = 0;

	Device->CreateShaderResourceView(CoverageTexture.Get(), &CoverageTextureResourceViewDesc, &CoverageTextureResourceView);

	// -----------------------------------------------------------------

	Microsoft::WRL::ComPtr<ID3D11SamplerState> CoverageSamplerState;

	D3D11_SAMPLER_DESC CoverageSamplerDesc;
	CoverageSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	CoverageSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	CoverageSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	CoverageSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	CoverageSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	CoverageSamplerDesc.MaxAnisotropy = 16;
	CoverageSamplerDesc.MinLOD = 0;
	CoverageSamplerDesc.MaxLOD = 1;
	CoverageSamplerDesc.MipLODBias = 0;

	Device->CreateSamplerState(&CoverageSamplerDesc, &CoverageSamplerState);

	// ----------------------------------------------------------------

	unsigned char* WorleyTextureBlob;
	int WorleyImageWidth, WorleyImageHeight, WorleyImageNumberOfChannels;

	WorleyTextureBlob = stbi_load("../Content/T_WorleyNoise.png",
		&WorleyImageWidth, &WorleyImageHeight, &WorleyImageNumberOfChannels, 0);

	Microsoft::WRL::ComPtr<ID3D11Texture3D> WorleyTexture;

	D3D11_TEXTURE3D_DESC WorleyTextureDescription;
	WorleyTextureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	WorleyTextureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	WorleyTextureDescription.Height = 128;
	WorleyTextureDescription.Width = 128;
	WorleyTextureDescription.Depth = 128;
	WorleyTextureDescription.CPUAccessFlags = 0;
	WorleyTextureDescription.MiscFlags = 0;
	WorleyTextureDescription.Usage = D3D11_USAGE_DEFAULT;
	WorleyTextureDescription.MipLevels = 1;

	D3D11_SUBRESOURCE_DATA WorleyTextureData;
	WorleyTextureData.pSysMem = WorleyTextureBlob;
	WorleyTextureData.SysMemPitch = sizeof(char) * WorleyImageNumberOfChannels * 128;
	WorleyTextureData.SysMemSlicePitch = sizeof(char) * WorleyImageNumberOfChannels * 128 * 128;

	Device->CreateTexture3D(&WorleyTextureDescription, &WorleyTextureData, &WorleyTexture);

	// ----------------------------------------------------------------

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> WorleyTextureResourceView;

	D3D11_SHADER_RESOURCE_VIEW_DESC WorleyTextureResourceViewDesc;
	WorleyTextureResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	WorleyTextureResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	WorleyTextureResourceViewDesc.Texture3D.MipLevels = 1;
	WorleyTextureResourceViewDesc.Texture3D.MostDetailedMip = 0;

	Device->CreateShaderResourceView(WorleyTexture.Get(), &WorleyTextureResourceViewDesc, &WorleyTextureResourceView);

	// -------------------------------------------------------------------

	unsigned char* WorleyDetailTextureBlob;
	int WorleyDetailImageWidth, WorleyDetailImageHeight, WorleyDetailImageNumberOfChannels;

	WorleyDetailTextureBlob = stbi_load("../Content/T_WorleyNoise_Detail.png",
		&WorleyDetailImageWidth, &WorleyDetailImageHeight, &WorleyDetailImageNumberOfChannels, 0);

	Microsoft::WRL::ComPtr<ID3D11Texture3D> WorleyDetailTexture;

	D3D11_TEXTURE3D_DESC WorleyDetailTextureDescription;
	WorleyDetailTextureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	WorleyDetailTextureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	WorleyDetailTextureDescription.Height = 32;
	WorleyDetailTextureDescription.Width = 32;
	WorleyDetailTextureDescription.Depth = 32;
	WorleyDetailTextureDescription.CPUAccessFlags = 0;
	WorleyDetailTextureDescription.MiscFlags = 0;
	WorleyDetailTextureDescription.Usage = D3D11_USAGE_DEFAULT;
	WorleyDetailTextureDescription.MipLevels = 1;

	D3D11_SUBRESOURCE_DATA WorleyDetailTextureData;
	WorleyDetailTextureData.pSysMem = WorleyDetailTextureBlob;
	WorleyDetailTextureData.SysMemPitch = sizeof(char) * WorleyDetailImageNumberOfChannels * 32;
	WorleyDetailTextureData.SysMemSlicePitch = sizeof(char) * WorleyDetailImageNumberOfChannels * 32 * 32;

	Device->CreateTexture3D(&WorleyDetailTextureDescription, &WorleyDetailTextureData, &WorleyDetailTexture);

	// ----------------------------------------------------------------

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> WorleyDetailTextureResourceView;

	D3D11_SHADER_RESOURCE_VIEW_DESC WorleyDetailTextureResourceViewDesc;
	WorleyDetailTextureResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	WorleyDetailTextureResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	WorleyDetailTextureResourceViewDesc.Texture3D.MipLevels = 1;
	WorleyDetailTextureResourceViewDesc.Texture3D.MostDetailedMip = 0;

	Device->CreateShaderResourceView(WorleyDetailTexture.Get(), &WorleyDetailTextureResourceViewDesc, &WorleyDetailTextureResourceView);

	// -------------------------------------------------------------------

	D3D11_RENDER_TARGET_BLEND_DESC TargetBlendDesc;
	TargetBlendDesc.BlendEnable = true;
	TargetBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	TargetBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	TargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	TargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	TargetBlendDesc.DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	TargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	TargetBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D11_BLEND_DESC BlendDesc;
	BlendDesc.AlphaToCoverageEnable = false;
	BlendDesc.IndependentBlendEnable = false;
	BlendDesc.RenderTarget[0] = TargetBlendDesc;

	Microsoft::WRL::ComPtr<ID3D11BlendState> BlendState;
	Device->CreateBlendState(&BlendDesc, &BlendState);

	DeviceContext->OMSetBlendState(BlendState.Get(), NULL, 0xffffffff);

	// -------------------------------------------------------------------

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(MainWindow.GetHandle());
	ImGui_ImplDX11_Init(Device.Get(), DeviceContext.Get());

	bool show_demo_window = true;

	// -------------------------------------------------------------------

	while (MainWindow.IsOpen())
	{
		DeviceContext->ClearRenderTargetView(BackBufferView.Get(), ClearColor);
		DeviceContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// ----------------------------------------------------------------

		if (MainWindow.IsRightClickDown())
		{
			float CameraPitchOffset = MouseSpeed * -MainWindow.MouseDeltaY;
			float CameraYawOffset = MouseSpeed * MainWindow.MouseDeltaX;

			Rotatorf DeltaRotation = Rotatorf(CameraPitchOffset, CameraYawOffset, 0.0f);
			CameraRotation = Rotatorf::CombineRotators(CameraRotation, DeltaRotation);

			float CameraPitchClamped = CameraRotation.Pitch < 180.0f ?
				Math::Clamp<float>(CameraRotation.Pitch, 1.0f, 89.0f) : Math::Clamp<float>(CameraRotation.Pitch, 271.0f, 359.0f);

			CameraRotation = Rotatorf(CameraPitchClamped, CameraRotation.Yaw, CameraRotation.Roll);
		}
		std::cout << CameraRotation.ToString() << std::endl;

		Vector3f CameraForwardVector = CameraRotation.Vector();
		Vector3f CameraRightVector = Vector3f::CrossProduct(Vector3f::UpVector, CameraForwardVector);

		Vector3f CameraForwardOffset = CameraForwardVector * MainWindow.GetUpValue() * CameraSpeed;
		Vector3f CameraRightOffset = CameraRightVector * MainWindow.GetRightValue() * CameraSpeed;

		CameraPosition = CameraPosition + CameraForwardOffset + CameraRightOffset;
		std::cout << CameraPosition.ToString() << std::endl;
		
		MainWindow.Tick(1);

		// ----------------------------------------------------------------

		auto Time = std::chrono::high_resolution_clock();
		auto Now =  Time.now();

		float TimeSeconds = (float)std::chrono::duration_cast<std::chrono::milliseconds>(Now.time_since_epoch()).count();

		Matrix<float> CameraViewMatrix = Math::LookAt(CameraPosition, CameraForwardVector, Vector3f::UpVector);

 		Matrix<float> ProjectionMatrix = PerspectiveMatrix<float>(90.0f,
  			(float)WindowWidth / (float)WindowHeight, 0.1f, 1000.0f);

		VsConstantBL.ViewMatrix = CameraViewMatrix;
		VsConstantBL.ProjectionMatrix = ProjectionMatrix;

		// ----------------------------------------------------------------

		const UINT stride = sizeof(Vertex);
		const UINT offset = 0;
		DeviceContext->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);
		DeviceContext->IASetInputLayout(VertexInputLayout.Get());
		DeviceContext->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		Matrix<float> CubeTransformMatrix = ScaleRotationTranslationMatrix<float>(Vector3f(1.0f), Rotatorf::ZeroRotator, Vector3f(0.0f, 0.0, 5.0f));
		VsConstantBL.TransformMatrix = CubeTransformMatrix;

		VSConstantData.pSysMem = &VsConstantBL;
		Device->CreateBuffer(&VsConstantBufferDesc, &VSConstantData, &VsConstantBuffer);
		DeviceContext->VSSetConstantBuffers(0, 1, VsConstantBuffer.GetAddressOf());

		DeviceContext->PSSetShader(PixelShader.Get(), 0, 0);

		DeviceContext->DrawIndexed(36, 0, 0);

		// -----------------------------------------------------------------

		DeviceContext->IASetVertexBuffers(0, 1, SkyVertexBuffer.GetAddressOf(), &stride, &offset);
		DeviceContext->IASetInputLayout(SkyVertexInputLayout.Get());
		DeviceContext->IASetIndexBuffer(SkyIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		Vector3f SkyPosition = CameraPosition + CameraForwardVector * 1.0f;
		Rotatorf SkyRotation = CameraRotation;
		Vector3f SkyScale = Vector3f(10.0f);

		Matrix<float> SkyTransformMatrix = ScaleRotationTranslationMatrix<float>(SkyScale, SkyRotation, SkyPosition);
		VsConstantBL.TransformMatrix = SkyTransformMatrix;

		VSConstantData.pSysMem = &VsConstantBL;
		Device->CreateBuffer(&VsConstantBufferDesc, &VSConstantData, &VsConstantBuffer);
		DeviceContext->VSSetConstantBuffers(0, 1, VsConstantBuffer.GetAddressOf());

		DeviceContext->PSSetShader(SkyPixelShader.Get(), 0, 0);

		DeviceContext->PSSetShaderResources(0, 1, CoverageTextureResourceView.GetAddressOf());
		DeviceContext->PSSetShaderResources(1, 1, WorleyTextureResourceView.GetAddressOf());
		DeviceContext->PSSetShaderResources(2, 1, WorleyDetailTextureResourceView.GetAddressOf());
		DeviceContext->PSSetSamplers(0, 1, CoverageSamplerState.GetAddressOf());


		PsConstantBL.CameraPosition = CameraPosition;
		PSConstantData.pSysMem = &PsConstantBL;

		Device->CreateBuffer(&PsConstantBufferDesc, &PSConstantData, &PsConstantBuffer);
		DeviceContext->PSSetConstantBuffers(0, 1, PsConstantBuffer.GetAddressOf());


		PSCloudData.pSysMem = &PsCloudBL;
		Device->CreateBuffer(&PsCloudBufferDesc, &PSCloudData, &PsCloudBuffer);
		DeviceContext->PSSetConstantBuffers(1, 1, PsCloudBuffer.GetAddressOf());

		DeviceContext->DrawIndexed(6, 0, 0);

		// -----------------------------------------------------------------

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 		if (show_demo_window)
		// 			ImGui::ShowDemoWindow(&show_demo_window);

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
		ImGui::SliderFloat("DetailNoiseScale", &PsCloudBL.DetailNoiseScale, 500.0f, 10000.0f, "%1f");
		ImGui::SliderFloat("DetailNoiseIntensity", &PsCloudBL.DetailNoiseIntensity, 0.0f, 1.0f, "%1f");
		ImGui::SliderFloat("Anvil", &PsCloudBL.Anvil, 0.0f, 1.0f, "%1f");
		ImGui::SliderFloat("TracingStartMaxDistance", &PsCloudBL.TracingStartMaxDistance, 100000.0, 500000.0, "%1f");

		ImGui::End();


		ImGui::Render();
		DeviceContext->OMSetRenderTargets(1, BackBufferView.GetAddressOf(), NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// -----------------------------------------------------------------

		SwapChain->Present(1, 0);
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	VertexShader.Reset();
	VertexShaderBlob.Reset();
	PixelShader.Reset();
	PixelShaderBlob.Reset();

	VertexBuffer.Reset();
	VertexInputLayout.Reset();

	BackBufferView.Reset();
	BackBuffer.Reset();

	SwapChain.Reset();
	DeviceContext.Reset();
	Device.Reset();

	Window::ShutDown();

	return 0;
}
