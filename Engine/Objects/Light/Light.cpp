#include "Light.h"
#include "Managers/ImGui/ImGuiManager.h" 

void Light::Initialize(DirectXCommon* dxCommon, Type type)
{
	//ライトの型を決める(現状平行光源)
	type_ = type;

	//ライト用のリソースを作成
	lightResource_ =
		CreateBufferResource(dxCommon->GetDevice(), sizeof(DirectionalLight));

	//ライトのデータにマップ
	lightResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightData_));

	//defaultの設定で初期化
	SetDefaultSettings();
}

void Light::SetDefaultSettings()
{
	// デフォルト設定
	lightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白色
	lightData_->direction = { 0.0f, -1.0f, 0.0f }; // 上から下方向
	lightData_->intensity = 1.0f; // 強度1.0
}

void Light::ImGui(const std::string& label)
{
#ifdef _DEBUG

	if (ImGui::TreeNode(label.c_str())) {
		// ライトの色
		if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&lightData_->color.x))) {

		}

		// ライトの方向（平行光源の場合）
		if (type_ == Type::DIRECTIONAL) {
			if (ImGui::DragFloat3("Direction", &lightData_->direction.x, 0.01f, -1.0f, 1.0f)) {

			}
		}

		// ライトの強度
		if (ImGui::DragFloat("Intensity", &lightData_->intensity, 0.01f, 0.0f, 10.0f)) {

		}

		// ライトの種類表示（読み取り専用）
		const char* typeNames[] = { "Directional", "Point", "Spot" };
		ImGui::Text("Type: %s", typeNames[static_cast<int>(type_)]);

		ImGui::TreePop();
	}

#endif
}
