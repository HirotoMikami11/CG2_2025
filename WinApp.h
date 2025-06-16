#pragma once
#include <Windows.h>
#include "MyFunction.h"

class WinApp
{

public:

	WinApp();
	~WinApp();

	void Initialize();
	void Finalize();
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	bool ProsessMessege();
	//ゲッター
	HWND GetHwnd() { return hwnd; };
	HINSTANCE GetInstance() const { return wc.hInstance; }
private:
	HWND hwnd = nullptr;
	WNDCLASS wc{};
};

