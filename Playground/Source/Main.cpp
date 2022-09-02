#pragma once

#include "pch.h"
#include "Window/Window.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, int nCmdShow)
{
	Window::Startup();

	Window MainWindow(L"DxWindow", 640, 480);


	ID3D11Device* Device;
	IDXGISwapChain* SwapChain;
	ID3D11DeviceContext* DeviceContext;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferDesc.Width = 0;
	SwapChainDesc.BufferDesc.Height = 0;
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

	HRESULT R = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
		&SwapChainDesc, &SwapChain, &Device, nullptr, &DeviceContext);

	if (FAILED(R))
	{
		return -1;
	}
	
	// -----------------------------------------------

	ID3D11Resource* BackBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> BackBufferView;

	SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&BackBuffer));
	Device->CreateRenderTargetView(BackBuffer, nullptr, &BackBufferView);
	float ClearColor[] = { 0.47f, 0.78f, 0.89f, 1.0f };

	// ------------------------------------------------

	struct Vertex
	{
		float X;
		float Y;
	};

	const Vertex vertecies[] = 
	{
		{0.0f, 0.5f},
		{0.5f, -0.5f},
		{-0.5f, -0.5f}
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
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	Device->CreateInputLayout(VertexInputElementDesc, (UINT)std::size(VertexInputElementDesc),
		VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), &VertexInputLayout);

	DeviceContext->IASetInputLayout(VertexInputLayout.Get());

	// ------------------------------------------------

	Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob> PixelShaderBlob;

	D3DReadFileToBlob(L"../Bin/Debug-windows-x86_64/playground/PixelShader.cso", &PixelShaderBlob);
	Device->CreatePixelShader(PixelShaderBlob->GetBufferPointer(), PixelShaderBlob->GetBufferSize(), nullptr, &PixelShader);

	DeviceContext->PSSetShader(PixelShader.Get(), 0, 0);
	DeviceContext->OMSetRenderTargets(1, BackBufferView.GetAddressOf(), nullptr);

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

	while (MainWindow.IsOpen())
	{
		MainWindow.Tick(1);

		DeviceContext->ClearRenderTargetView(BackBufferView.Get(), ClearColor);

		DeviceContext->Draw(3, 0);

		SwapChain->Present(1, 0);
	}

	if (DeviceContext)	{ DeviceContext->Release(); }
	if (SwapChain)		{ SwapChain->Release(); }
	if (Device)			{ Device->Release(); }
	if (BackBuffer)		{ BackBuffer->Release(); }
	if (BackBufferView)	{ BackBufferView->Release(); }

	Window::ShutDown();

	return 0;
}