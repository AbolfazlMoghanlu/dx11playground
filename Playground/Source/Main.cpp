#pragma once

#include "pch.h"
#include "Window/Window.h"
#include <time.h>

Rotatorf CameraRotation = Rotatorf(0.0f);
const float MouseSpeed = 500.0f;
const float CameraSpeed = 100.0f;

Vector3f CameraPosition = Vector3f(0.0f);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, int nCmdShow)
{
	Window::Startup();

	Window MainWindow(L"DxWindow", 640, 480);


	AllocConsole();
	
	static std::ofstream conout("CONOUT$", std::ios::out);
	// Set std::cout stream buffer to conout's buffer (aka redirect/fdreopen)
	std::cout.rdbuf(conout.rdbuf());
	

	Microsoft::WRL::ComPtr<ID3D11Device> Device;
	Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferDesc.Width = 640;
	SwapChainDesc.BufferDesc.Height = 480;
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

// 	const Vertex vertecies[] = 
// 	{
// 		{0.5f, 0.5f, 0.2f,		1.0f, 1.0f, 1.0f},
// 		{-0.5f, -0.5f, 0.2f,	1.0f, 0.0f, 0.0f},
// 		{0.5f, -0.5f, 0.2f,		0.0f, 1.0f, 0.0f},
// 		{-0.5f, 0.5f, 0.2f,		0.0f, 0.0f, 1.0f},
// 	};
// 
// 	const unsigned short int Indices[] =
// 	{
// 		0, 2, 1,
// 		0, 1, 3
// 	};


	const Vertex vertecies[] = 
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

	const unsigned short int Indices[] =
	{
		4, 0, 1, 4, 1, 5, // front
		6, 2, 0, 6, 0, 4, // top
		0, 2, 3, 0, 3, 1, // right
		6, 4, 5, 6, 5, 7, // left
		5, 1, 3, 5, 3, 7, // bottom
		6, 2, 3, 6, 3, 7  // back
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	D3D11_BUFFER_DESC VertexBufferDesc;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.CPUAccessFlags = false;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.ByteWidth = sizeof(vertecies);
	VertexBufferDesc.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA VertexDataDesc;
	VertexDataDesc.pSysMem = vertecies;

	Device->CreateBuffer(&VertexBufferDesc, &VertexDataDesc, &VertexBuffer);

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);

	// ------------------------------------------------

	Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> VertexShaderBlob;

	D3DReadFileToBlob(L"../Bin/Debug-windows-x86_64/playground/VertexShader.cso", VertexShaderBlob.GetAddressOf());
	Device->CreateVertexShader(VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), nullptr, &VertexShader);

	DeviceContext->VSSetShader(VertexShader.Get(), 0, 0);

	// ------------------------------------------------

	Microsoft::WRL::ComPtr<ID3D11InputLayout> VertexInputLayout;
	const D3D11_INPUT_ELEMENT_DESC VertexInputElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	Device->CreateInputLayout(VertexInputElementDesc, (UINT)std::size(VertexInputElementDesc),
		VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), VertexInputLayout.GetAddressOf());

	DeviceContext->IASetInputLayout(VertexInputLayout.Get());

	// ------------------------------------------------

	D3D11_BUFFER_DESC IndexBufferDesc;
	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.ByteWidth = sizeof(Indices);
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = false;
	IndexBufferDesc.MiscFlags = 0;
	IndexBufferDesc.StructureByteStride = sizeof(unsigned short int);

	D3D11_SUBRESOURCE_DATA IndexData;
	IndexData.pSysMem = Indices;

	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
	Device->CreateBuffer(&IndexBufferDesc, &IndexData, &IndexBuffer);

	DeviceContext->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	// ------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob> PixelShaderBlob;

	D3DReadFileToBlob(L"../Bin/Debug-windows-x86_64/playground/PixelShader.cso", &PixelShaderBlob);
	Device->CreatePixelShader(PixelShaderBlob->GetBufferPointer(), PixelShaderBlob->GetBufferSize(), nullptr, &PixelShader);

	DeviceContext->PSSetShader(PixelShader.Get(), 0, 0);
	//DeviceContext->OMSetRenderTargets(1, BackBufferView.GetAddressOf(), nullptr);

	// ------------------------------------------------

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ------------------------------------------------

	D3D11_VIEWPORT ViewportDesc;
	ViewportDesc.Width = 640;
	ViewportDesc.Height = 480;
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

	D3D11_TEXTURE2D_DESC DepthDesc;
	DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthDesc.Width = 640;
	DepthDesc.Height = 480;
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

	while (MainWindow.IsOpen())
	{
		if (MainWindow.IsRightClickDown())
		{
			Rotatorf DeltaRotation = Rotatorf(MouseSpeed * MainWindow.MouseDeltaY, MouseSpeed * MainWindow.MouseDeltaX, 0.0f);
			CameraRotation = Rotatorf::CombineRotators(CameraRotation, DeltaRotation);
		}
		std::cout << CameraRotation.ToString() << std::endl;


		CameraPosition += Vector3f(MainWindow.GetRightValue() * CameraSpeed, MainWindow.GetUpValue() * CameraSpeed, 0.0f);
		std::cout << CameraPosition.ToString() << std::endl;
		
		MainWindow.Tick(1);

		DeviceContext->ClearRenderTargetView(BackBufferView.Get(), ClearColor);
		DeviceContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// ----------------------------------------------------------------

		auto Time = std::chrono::high_resolution_clock();
		auto Now =  Time.now();

		float TimeSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(Now.time_since_epoch()).count();
		

		Matrix<float> TransformMatrix = ScaleTranslationMatrix<float>(Vector3f(0.0, 0.0, 0.5f), Vector3f(0.25f));

		Vector3f CameraForwardVector = CameraRotation.Vector();

		Matrix<float> CameraViewMatrix = Math::LookAt(CameraPosition, CameraForwardVector, Vector3f::UpVector);


		VsConstantBL.TransformMatrix = TransformMatrix;
		VsConstantBL.ViewMatrix = CameraViewMatrix;
		VSConstantData.pSysMem = &VsConstantBL;
		Device->CreateBuffer(&VsConstantBufferDesc, &VSConstantData, &VsConstantBuffer);

		DeviceContext->VSSetConstantBuffers(0, 1, VsConstantBuffer.GetAddressOf());

		// ----------------------------------------------------------------

		DeviceContext->DrawIndexed(36, 0, 0);

		SwapChain->Present(1, 0);
	}

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
