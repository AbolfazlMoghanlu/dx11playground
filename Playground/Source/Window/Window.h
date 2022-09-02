#pragma once

const DWORD DefaultStyle = WS_CAPTION | WS_MINIMIZE | WS_SYSMENU;

class Window
{
public:
	Window(LPCWSTR Title, int SizeX, int SizeY, DWORD Style = DefaultStyle);
	~Window();

	static void Startup();
	static void ShutDown();

	void Tick(float DeltaTime);

	static void RegisterWindowClass();
	static void UnregisterWindowClass();


	bool IsOpen() const;

protected:
	virtual LRESULT HandleMessege(HWND Hanlde, UINT msg, WPARAM Wparam, LPARAM Lparam);

private:
	static LRESULT CALLBACK SetupWindowProc(HWND Hanlde, UINT msg, WPARAM Wparam, LPARAM Lparam);
	static LRESULT CALLBACK WindowProcAbs(HWND Hanlde, UINT msg, WPARAM Wparam, LPARAM Lparam);

	static WNDCLASSEX WindowClass;
	static HINSTANCE HInstance;

	HWND WindowHandle;

	bool bOpen;
};

