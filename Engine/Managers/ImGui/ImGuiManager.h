#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>

#include "BaseSystem/WinApp/WinApp.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"

#ifdef _DEBUG

// ImGui関連のインクルード
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//imguiの便利関数をまとめたクラス
#include "Managers/ImGui/MyImGui.h"

#endif

// 前方宣言
class WinApp;
class DirectXCommon;

/// <summary>
/// ImGuiの管理クラス
/// </summary>
class ImGuiManager {
public:
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>ImGuiManagerのインスタンス</returns>
	static ImGuiManager* GetInstance();

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="winApp">ウィンドウアプリケーション</param>
	/// <param name="directXCommon">DirectX基盤</param>
	void Initialize(WinApp* winApp, DirectXCommon* directXCommon);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// ImGuiの受け付け開始
	/// </summary>
	void Begin();

	/// <summary>
	/// ImGuiの受け付け終了
	/// </summary>
	void End();

	/// <summary>
	/// 画面への描画
	/// </summary>
	/// <param name="commandList">コマンドリスト</param>
	void Draw(ID3D12GraphicsCommandList* commandList);


	/// <summary>
	/// シーンの名前を表示する
	/// </summary>
	void SceneName(const char* SceneName);

private:
	/// <summary>
	/// ImGuiのスタイル設定
	/// </summary>
	void SetupImGuiStyle();
	
	// シングルトンパターンのため、コンストラクタをprivateに
	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;
};