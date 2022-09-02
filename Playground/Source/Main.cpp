#pragma once

#include "pch.h"
#include "Window/Window.h"
#include <d3d11.h>

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

	// -----------------------------------------------

	ID3D11Resource* BackBuffer = nullptr;
	ID3D11RenderTargetView* BackBufferView = nullptr;

	SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&BackBuffer));
	Device->CreateRenderTargetView(BackBuffer, nullptr, &BackBufferView);
	float ClearColor[] = { 0.47f, 0.78f, 0.89f, 1.0f };

	if (FAILED(R))
	{
		return -1;
	}
	
	while (MainWindow.IsOpen())
	{
		MainWindow.Tick(1);


		DeviceContext->ClearRenderTargetView(BackBufferView, ClearColor);
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