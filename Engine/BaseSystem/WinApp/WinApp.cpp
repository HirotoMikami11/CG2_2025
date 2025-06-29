#include "BaseSystem/WinApp/WinApp.h"
#ifdef _DEBUG
///ImGui
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

WinApp::WinApp() {



}
WinApp::~WinApp() {

}

void WinApp::Initialize(const std::wstring& title) {
	//COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);

	///
	/// ウィンドウクラスを登録する
	///

	
	//ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;

	//ウィンドウクラス名
	wc.lpszClassName = L"CG2WindowClass";
	//インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウィンドウクラスを登録する
	RegisterClass(&wc);




	//　ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,GraphicsConfig::kClientWidth,GraphicsConfig::kClientHeight };

	//クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウの生成
	hwnd = CreateWindow(
		wc.lpszClassName,		//利用するクラス名
		title.c_str(),					//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,	//よく見るウィンドウスタイル
		CW_USEDEFAULT,			//表示X座標
		CW_USEDEFAULT,			//表示Y座標
		wrc.right - wrc.left,	//ウィンドウの横幅
		wrc.bottom - wrc.top,	//ウィンドウの縦幅
		nullptr,				//親ウィンドウハンドル
		nullptr,				//メニューハンドル
		wc.hInstance,			//インスタンスハンドル
		nullptr);				//オプション

	//ウィンドウを表示する関数
	ShowWindow(hwnd, SW_SHOW);

}

void WinApp::Finalize() {
	CloseWindow(hwnd);
}

LRESULT CALLBACK WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

#ifdef _DEBUG
	//ImGuiにメッセージを渡し、マウスやキーボードで操作できるようにする
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif



	//メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対してアプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}
	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

bool WinApp::ProsessMessege()
{
	MSG msg{};
	//Windowにメッセージが来てたら最優先で処理
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.message != WM_QUIT;
}


