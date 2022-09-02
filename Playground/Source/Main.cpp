#pragma once

#include "pch.h"
#include "Window/Window.h"

/*
LRESULT CALLBACK WindowsProcedual(HWND Hanlde, UINT msg, WPARAM Wparam, LPARAM Lparam)
{
	switch (msg)
	{
		case WM_CLOSE:
			PostQuitMessage(1);
			break;

		case WM_KEYDOWN:
			if (Wparam == 'F')
			{
				
			}
			break;

		case WM_CHAR:
			break;

		case WM_LBUTTONDOWN:
			POINTS pt = MAKEPOINTS(Lparam);
			std::wstringstream ss;
			ss << pt.x << " , " << pt.y;
			SetWindowText(Hanlde, ss.str().c_str());
			break;
	}

	return DefWindowProc(Hanlde, msg, Wparam, Lparam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, int nCmdShow)
{
	const LPCWSTR WindowClassName = L"Dx11Playground";

	WNDCLASSEX WindowClass;
	WindowClass.cbSize = sizeof(WindowClass);
	WindowClass.style = CS_OWNDC;
	WindowClass.lpfnWndProc = WindowsProcedual;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = hInstance;
	WindowClass.hCursor = nullptr;
	WindowClass.hbrBackground = nullptr;
	WindowClass.hIcon = nullptr;
	WindowClass.hIconSm = nullptr;
	WindowClass.lpszMenuName = nullptr;
	WindowClass.lpszClassName = WindowClassName;

	RegisterClassEx(&WindowClass);


	DWORD StyleFlag = WS_CAPTION | WS_MINIMIZE | WS_SYSMENU;

	HWND WindowHandle = CreateWindowEx(0, WindowClassName, L"Dx11", StyleFlag,
		0, 0, 640, 480, nullptr, nullptr, hInstance, nullptr);

	ShowWindow(WindowHandle, SW_SHOW);

	MSG msg;
	BOOL bResult;
	while (bResult = GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);

		DispatchMessage(&msg);
	}

	if (bResult == -1)
	{
		std::cout << "Error!";
		return -1;
	}

	else
	{
		return (int)msg.wParam;
	}
}
*/

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, int nCmdShow)
{
	Window::Startup();

	Window MainWindow(L"DxWindow", 640, 480);
	
	while (MainWindow.IsOpen())
	{
		MainWindow.Tick(1);
	}

	Window::ShutDown();

	return 0;
}