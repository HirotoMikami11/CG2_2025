#define NOMINMAX // C+標準のstd::maxを使えるようにするため(windows.hが上書きしてしまっている)
#include "GridLine.h"
#include "Managers/ImGuiManager.h"
#include "Managers/Texture/TextureManager.h"
#include <algorithm>
#include <cmath>

void GridLine::Initialize(DirectXCommon* dxCommon)
{
	directXCommon_ = dxCommon;

	// Lineクラスを静的初期化（一度だけ）
	Line::InitializeStatic(dxCommon);

	// Transformを初期化
	transform_.Initialize(dxCommon);

	// デフォルトでグリッドを作成
	CreateGrid();
}

void GridLine::CreateGrid(
	float size,
	float interval,
	float majorInterval,
	const Vector4& normalColor,
	const Vector4& majorColor)
{
	Clear();

	// 設定を保存
	gridSize_ = size;
	gridInterval_ = interval;
	gridMajorInterval_ = majorInterval;
	gridNormalColor_ = normalColor;
	gridMajorColor_ = majorColor;

	float halfSize = size * 0.5f;

	// X方向の線（Z軸に沿って）
	for (float x = -halfSize; x <= halfSize; x += interval) {
		Vector4 color;
		if (std::abs(x) < 0.001f) {//誤差で0にならない時のために0.001にしておく
			//原点のZ軸は青色に変更する
			color = Vector4{ 0.0f, 0.0f, 1.0f, 1.0f };
		} else if (std::fmod(std::abs(x), majorInterval) < 0.001f) {
			color = majorColor;
		} else {
			color = normalColor;
		}

		Vector3 start = { x, 0.0f, -halfSize };
		Vector3 end = { x, 0.0f, halfSize };
		AddLine(start, end, color);
	}

	// Z方向の線（X軸に沿って）
	for (float z = -halfSize; z <= halfSize; z += interval) {
		Vector4 color;

		if (std::abs(z) < 0.001f) {//誤差で0にならない時のために0.001にしておく
			//原点のX軸は赤色に変更する
			color = Vector4{ 1.0f, 0.0f, 0.0f, 1.0f };
		} else if (std::fmod(std::abs(z), majorInterval) < 0.001f) {
			color = majorColor;
		} else {
			color = normalColor;
		}

		Vector3 start = { -halfSize, 0.0f, z };
		Vector3 end = { halfSize, 0.0f, z };
		AddLine(start, end, color);
	}
}

void GridLine::AddLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	auto line = std::make_unique<Line>();
	line->Initialize();
	line->SetPoints(start, end);
	line->SetColor(color);
	lines_.push_back(std::move(line));

}

void GridLine::Clear()
{
	lines_.clear();
}

void GridLine::Update(const Matrix4x4& viewProjectionMatrix)
{
	if (!isActive_) {
		return;
	}

	// Transform行列を更新
	transform_.UpdateMatrix(viewProjectionMatrix);


	for (auto& line : lines_) {
		line->Update(viewProjectionMatrix);
	}


}

void GridLine::Draw(const Matrix4x4& viewProjectionMatrix)
{
	if (!isVisible_ || !isActive_ || lines_.empty()) {
		return;
	}
	DrawIndividual(viewProjectionMatrix);

}

void GridLine::DrawIndividual(const Matrix4x4& viewProjectionMatrix)
{
	if (!isVisible_ || !isActive_ || lines_.empty()) {
		return;
	}

	// 各線分を個別に描画（色が正確に反映される）
	for (auto& line : lines_) {
		if (line->IsVisible()) {
			line->Draw(viewProjectionMatrix);
		}
	}
}

void GridLine::ImGui()
{
#ifdef _DEBUG
	if (ImGui::TreeNode(name_.c_str())) {
		// 基本設定
		ImGui::Checkbox("Visible", &isVisible_);
		ImGui::Checkbox("Active", &isActive_);

		ImGui::Text("Line Count: %zu", lines_.size());

		// Transform
		if (ImGui::CollapsingHeader("Transform")) {
			Vector3 position = transform_.GetPosition();
			Vector3 rotation = transform_.GetRotation();
			Vector3 scale = transform_.GetScale();

			if (ImGui::DragFloat3("Position", &position.x, 0.01f)) {
				transform_.SetPosition(position);
			}
			if (ImGui::DragFloat3("Rotation", &rotation.x, 0.01f)) {
				transform_.SetRotation(rotation);
			}
			if (ImGui::DragFloat3("Scale", &scale.x, 0.01f)) {
				transform_.SetScale(scale);
			}
		}

		// グリッド設定
		if (ImGui::CollapsingHeader("Grid Settings")) {
			bool gridChanged = false;

			if (ImGui::DragFloat("Grid Size", &gridSize_, 1.0f, 10.0f, 200.0f)) {
				gridChanged = true;
			}

			if (ImGui::DragFloat("Grid Interval", &gridInterval_, 0.1f, 0.1f, 5.0f)) {
				gridChanged = true;
			}

			if (ImGui::DragFloat("Major Interval", &gridMajorInterval_, 1.0f, 2.0f, 50.0f)) {
				gridChanged = true;
			}

			if (ImGui::ColorEdit4("Normal Color", &gridNormalColor_.x)) {
				gridChanged = true;
			}

			if (ImGui::ColorEdit4("Major Color", &gridMajorColor_.x)) {
				gridChanged = true;
			}

			if (ImGui::Button("Regenerate Grid") || gridChanged) {
				CreateGrid(gridSize_, gridInterval_, gridMajorInterval_, gridNormalColor_, gridMajorColor_);
			}
		}

		ImGui::TreePop();
	}
#endif
}
