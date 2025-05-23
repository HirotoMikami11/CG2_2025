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
	// 基本スタイルを設定
	ImGui::StyleColorsDark();

	// カスタムスタイルの設定例
	ImGuiStyle& style = ImGui::GetStyle();

	// パディング・間隔
	style.WindowPadding = ImVec2(8, 6);
	style.FramePadding = ImVec2(5, 3);
	style.ItemSpacing = ImVec2(6, 3);

	// 透明度
	style.Alpha = 1.00f;

	// カラーのカスタマイズ例
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.03f, 0.03f, 0.03f, 0.98f);
	colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.10f, 0.48f, 0.93f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);

}
