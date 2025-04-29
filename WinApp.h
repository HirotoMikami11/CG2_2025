#pragma once
#include <Windows.h>
#include "MyFunction.h"

///ImGui
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
private:
	HWND hwnd = nullptr;

};

