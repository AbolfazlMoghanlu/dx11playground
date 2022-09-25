#pragma once

const DWORD DefaultStyle = WS_CAPTION | WS_MINIMIZE | WS_SYSMENU;

class Window
{
public:
	Window(LPCWSTR InTitle, int InSizeX, int InSizeY, DWORD InStyle = DefaultStyle);
	~Window();

	static void Startup();
	static void ShutDown();

	void Tick(float DeltaTime);

	static void RegisterWindowClass();
	static void UnregisterWindowClass();


	bool IsOpen() const;
	HWND& GetHandle();
	void ForceClose();


	bool IsRightClickDown() const;

	float GetRightValue() const;
	float GetUpValue() const;

protected:
	virtual LRESULT HandleMessege(HWND Handle, UINT msg, WPARAM Wparam, LPARAM Lparam);

private:
	static LRESULT CALLBACK SetupWindowProc(HWND Hanlde, UINT msg, WPARAM Wparam, LPARAM Lparam);
	static LRESULT CALLBACK WindowProcAbs(HWND Hanlde, UINT msg, WPARAM Wparam, LPARAM Lparam);

	void UpdateCursorPosition();

	static WNDCLASSEX WindowClass;
	static HINSTANCE HInstance;

	HWND WindowHandle;

	bool bOpen = true;

	int SizeX = 0;
	int SizeY = 0;

public:
	float MouseLastX = 0.0f;
	float MouseLastY = 0.0f;

	float MouseDeltaX = 0.0f;
	float MouseDeltaY = 0.0f;
};

