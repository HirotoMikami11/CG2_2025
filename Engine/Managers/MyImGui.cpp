#include "MyImGui.h"
#include "../externals/imgui/imgui.h"

namespace myImGui {


	void CenterText(const char* text) {

		// テキストを中央に配置するためのオフセットを計算
		float windowWidth = ImGui::GetWindowSize().x;
		float textWidth = ImGui::CalcTextSize(text).x;
		float offsetX = (windowWidth - textWidth) * 0.5f;

		// 現在のY位置を取得（カーソル位置復元用）
		ImVec2 cursorPos = ImGui::GetCursorPos();

		// 中央にカーソルを設定して描画
		if (offsetX > 0.0f) { ImGui::SetCursorPosX(offsetX); }
		ImGui::TextUnformatted(text);

		// 左揃えに戻す（次の行のXを元の位置に）
		ImGui::SetCursorPosX(cursorPos.x);
	}


}