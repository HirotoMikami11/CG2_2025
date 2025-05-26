#include "ImGuiManager.h"


ImGuiManager* ImGuiManager::GetInstance() {
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* directXCommon) {
	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// プラットフォームとレンダラーの初期化
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(
		directXCommon->GetDevice(),
		directXCommon->GetSwapChainDesc().BufferCount,
		directXCommon->GetRTVDesc().Format,
		directXCommon->GetSRVDescriptorHeap(),
		directXCommon->GetSRVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		directXCommon->GetSRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart()
	);

	// スタイルを設定
	SetupImGuiStyle();

}

void ImGuiManager::Finalize() {
	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiManager::Begin() {
	// ImGuiにフレームが始まることを伝える
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End() {
	// ImGuiの内部コマンドを生成する
	ImGui::Render();
}

void ImGuiManager::Draw(ID3D12GraphicsCommandList* commandList) {
	//実際の directXCommon-> GetCommandList()のImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}









void ImGuiManager::SetupImGuiStyle()
{
	// 基本スタイル（Dark）を設定
	ImGui::StyleColorsDark();

	//// Cinder-ImGui標準風のスタイル設定
	//ImGuiStyle& style = ImGui::GetStyle();

	//// スタイルパラメータ
	//style.WindowRounding = 4.0f;
	//style.FrameRounding = 4.0f;
	//style.ScrollbarRounding = 6.0f;
	//style.GrabRounding = 4.0f;
	//style.TabRounding = 4.0f;
	//style.ChildRounding = 4.0f;
	//style.PopupRounding = 4.0f;

	//style.FramePadding = ImVec2(6, 4);
	//style.WindowPadding = ImVec2(8, 8);
	//style.ItemSpacing = ImVec2(8, 4);
	//style.ItemInnerSpacing = ImVec2(4, 4);

	//style.WindowBorderSize = 1.0f;
	//style.FrameBorderSize = 0.0f;
	//style.PopupBorderSize = 1.0f;
	//style.TabBorderSize = 0.0f;

	//style.Alpha = 1.00f;

	//// Cinder-ImGui風のカラーテーマ
	//ImVec4* colors = style.Colors;

	//// 背景：深いグレー系
	//colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);      // ダークグレー背景
	//colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);       // 子ウィンドウは少し暗く
	//colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.94f);       // ポップアップ

	//// テキスト：明るいグレー
	//colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);          // 明るいグレーテキスト
	//colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);  // 無効テキスト
	//colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f); // 青い選択背景

	//// ボーダー：暗いグレー
	//colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.30f, 0.80f);        // ボーダー
	//colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);  // 影無し

	//// フレーム：暗めのグレー
	//colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);       // フレーム背景
	//colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f); // ホバー時
	//colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f); // アクティブ時

	//// タイトルバー
	//colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);       // 非アクティブタイトル
	//colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f); // アクティブタイトル
	//colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f); // 折りたたみ

	//// メニューバー
	//colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);     // メニューバー

	//// スクロールバー
	//colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);   // スクロールバー背景
	//colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f); // つまみ
	//colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f); // ホバー
	//colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);  // アクティブ

	//// チェックマーク：青系アクセント
	//colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);     // 青いチェックマーク

	//// スライダー：青系アクセント
	//colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);    // スライダーつまみ
	//colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f); // アクティブ時

	//// ボタン：深いグレー + 青アクセント
	//colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);        // ボタン
	//colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f); // ホバー時
	//colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);  // アクティブ時

	//// ヘッダー：青系アクセント
	//colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);        // ヘッダー
	//colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f); // ホバー時
	//colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);  // アクティブ時

	//// セパレーター
	//colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);     // セパレーター
	//colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f); // ホバー時
	//colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);  // アクティブ時

	//// リサイズグリップ
	//colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);    // リサイズグリップ
	//colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f); // ホバー時
	//colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);  // アクティブ時

	//// タブ：グレー + 青アクセント
	//colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);           // 非アクティブタブ
	//colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);    // ホバー時
	//colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);     // アクティブタブ
	//colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);  // 非フォーカス
	//colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f); // 非フォーカスアクティブ

	//// プロット
	//colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);     // プロットライン
	//colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f); // ホバー時
	//colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f); // ヒストグラム
	//colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f); // ホバー時

	//// その他
	//colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f); // ドラッグドロップ
	//colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);  // ナビゲーション
	//colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	//colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.90f, 0.20f);
	//colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.90f, 0.35f); // モーダル背景
}