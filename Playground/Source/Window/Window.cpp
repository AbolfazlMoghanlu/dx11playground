#include "pch.h"
#include "Window.h"

#define WINDOW_CLASS_NAME L"DX11Playground"

WNDCLASSEX Window::WindowClass;
HINSTANCE Window::HInstance;

Window::Window(LPCWSTR InTitle, int InSizeX, int InSizeY, DWORD InStyle /*= DefaultStyle*/)
{
	SizeX = InSizeX;
	SizeY = InSizeY;

	RECT R = {0, 0, InSizeX, InSizeY};

	AdjustWindowRect(&R, InStyle, false);

	int AdjustedSizeX = R.right - R.left;
	int AdjustedSizeY = R.bottom - R.top;

	WindowHandle = CreateWindowEx(0, WINDOW_CLASS_NAME, InTitle, InStyle,
		0, 0, AdjustedSizeX, AdjustedSizeY, nullptr, nullptr, HInstance, this);

	ShowWindow(WindowHandle, SW_SHOW);
}

Window::~Window()
{
	if (WindowHandle)
	{
		DestroyWindow(WindowHandle);
	}
}

void Window::Startup()
{
	HINSTANCE Handle = GetModuleHandle(nullptr);
	RegisterWindowClass();
}

void Window::ShutDown()
{
	UnregisterWindowClass();
}

void Window::Tick(float DeltaTime)
{
	MSG msg;
	BOOL bResult;
	bResult = GetMessage(&msg, WindowHandle, 0, 0) > 0;

	if (bResult)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UpdateCursorPosition();
}

void Window::RegisterWindowClass()
{
	WNDCLASSEX WindowClass;

	WindowClass.cbSize = sizeof(WindowClass);
	WindowClass.style = CS_OWNDC;
	WindowClass.lpfnWndProc = SetupWindowProc;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = HInstance;
	WindowClass.hCursor = nullptr;
	WindowClass.hbrBackground = nullptr;
	WindowClass.hIcon = nullptr;
	WindowClass.hIconSm = nullptr;
	WindowClass.lpszMenuName = nullptr;
	WindowClass.lpszClassName = WINDOW_CLASS_NAME;

	RegisterClassEx(&WindowClass);
}

void Window::UnregisterWindowClass()
{
	UnregisterClass(WINDOW_CLASS_NAME, HInstance);
}

LRESULT CALLBACK Window::SetupWindowProc(HWND Hanlde, UINT msg, WPARAM Wparam, LPARAM Lparam)
{
	if (msg == WM_CREATE)
	{
		const CREATESTRUCT* const CreateStruct = reinterpret_cast<CREATESTRUCT*>(Lparam);
		Window* const window = static_cast<Window*>(CreateStruct->lpCreateParams);
		SetWindowLongPtr(Hanlde, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		SetWindowLongPtr(Hanlde, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::WindowProcAbs));
	}

	return DefWindowProc(Hanlde, msg, Wparam, Lparam);
}

LRESULT CALLBACK Window::WindowProcAbs(HWND Hanlde, UINT msg, WPARAM Wparam, LPARAM Lparam)
{
	LONG_PTR UserData = GetWindowLongPtr(Hanlde, GWLP_USERDATA);
	Window* Wind = reinterpret_cast<Window*>(UserData);

	return Wind->HandleMessege(Hanlde, msg, Wparam, Lparam);
}

void Window::UpdateCursorPosition()
{
	POINT P = POINT();
	GetCursorPos(&P);

	ScreenToClient(WindowHandle, &P);

	MouseLastX = Math::Clamp((float)P.x / SizeX, 0.0f, 1.0f);
	MouseLastY = Math::Clamp((float)P.y / SizeY, 0.0f, 1.0f);

	std::cout << MouseLastX << ", " << MouseLastY << std::endl;
}

bool Window::IsOpen() const
{
	return bOpen;
}

HWND& Window::GetHandle()
{
	return WindowHandle;
}

LRESULT Window::HandleMessege(HWND Hanlde, UINT msg, WPARAM Wparam, LPARAM Lparam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(1);
		DestroyWindow(WindowHandle);
		bOpen = false;
		break;

	case WM_KEYDOWN:
		if (Wparam == 'F')
		{

		}
		break;

	case WM_CHAR:
		break;

	case WM_LBUTTONDOWN:
		/*
		POINTS pt = MAKEPOINTS(Lparam);
		std::wstringstream ss;
		ss << pt.x << " , " << pt.y;
		SetWindowText(Hanlde, ss.str().c_str());
		*/
		break;
	}

	return DefWindowProc(Hanlde, msg, Wparam, Lparam);
	//return nullptr;
}
