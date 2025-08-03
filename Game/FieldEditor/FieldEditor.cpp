#define NOMINMAX
#include "FieldEditor.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "BaseSystem/Logger/Logger.h"

namespace {
	// 岩タイプ表示名の配列
	const char* kRockTypeDisplayNames[] = {
		"Rock1",
		"Rock2",
		"Rock3"
	};
	constexpr int kRockTypeCount = 3;
}

FieldEditor::FieldEditor() {
}

FieldEditor::~FieldEditor() {
}

void FieldEditor::Initialize(DirectXCommon* dxCommon, CameraController* cameraController) {
	directXCommon_ = dxCommon;
	cameraController_ = cameraController;

	// 新規追加用プレビューRockを作成
	CreateOrUpdatePreviewRock();
}

void FieldEditor::Update(const Matrix4x4& viewProjectionMatrix) {
	// エディタが無効なら早期リターン
	if (!isEditorEnabled_) {
		return;
	}

	// 配置済みRockの更新
	if (showPreviewRocks_) {
		for (auto& objData : objectDataList_) {
			if (objData.actualRock) {
				objData.actualRock->Update(viewProjectionMatrix);
			}
		}
	}

	// 新規追加用プレビューRockの更新
	if (previewRock_) {
		previewRock_->Update(viewProjectionMatrix);
	}
}

void FieldEditor::Draw(const Light& directionalLight) {
	// エディタが無効なら早期リターン
	if (!isEditorEnabled_) {
		return;
	}

	// 配置済みRockの描画
	if (showPreviewRocks_) {
		for (auto& objData : objectDataList_) {
			if (objData.actualRock) {
				objData.actualRock->Draw(directionalLight);
			}
		}
	}

	// 新規追加用プレビューRockの描画
	if (previewRock_) {
		previewRock_->Draw(directionalLight);
	}
}

void FieldEditor::ImGui() {
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Field Editor")) {
		// エディタのon/off
		ImGui::Checkbox("Enable Editor", &isEditorEnabled_);

		if (!isEditorEnabled_) {
			return;
		}

		ImGui::Separator();

		// 表示設定
		ImGui::Text("Display Settings:");
		ImGui::Checkbox("Show Preview Rocks", &showPreviewRocks_);

		ImGui::Separator();

		// ファイル操作
		ImGui::Text("File Operations:");
		ImGui::InputText("CSV File Path", csvFileName_, sizeof(csvFileName_));

		if (ImGui::Button("Save to CSV")) {
			if (SaveToCSV(csvFileName_)) {
				Logger::Log(Logger::GetStream(), "FieldEditor: CSV saved successfully\n");
			} else {
				Logger::Log(Logger::GetStream(), "FieldEditor: Failed to save CSV\n");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Load from CSV")) {
			if (LoadFromCSV(csvFileName_)) {
				Logger::Log(Logger::GetStream(), "FieldEditor: CSV loaded successfully\n");
			} else {
				Logger::Log(Logger::GetStream(), "FieldEditor: Failed to load CSV\n");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear All")) {
			ClearAllObjects();
		}

		ImGui::Separator();

		// 新規Rock追加セクション
		ImGui::Text("Add New Rock:");

		// カメラ位置取得とリセットボタン
		if (ImGui::Button("Get Camera Position")) {
			if (cameraController_) {
				editorPosition_ = cameraController_->GetPosition();
				CreateOrUpdatePreviewRock(); // プレビューRockを更新
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset Position")) {
			editorPosition_ = { 0.0f, 0.0f, 0.0f };
			CreateOrUpdatePreviewRock();
		}

		// 座標設定（リアルタイム更新）
		if (ImGui::DragFloat3("Position", &editorPosition_.x, 0.1f)) {
			CreateOrUpdatePreviewRock();
		}

		// 回転設定（リアルタイム更新）
		if (ImGui::DragFloat3("Rotation", &editorRotation_.x, 0.1f)) {
			CreateOrUpdatePreviewRock();
		}

		// スケール設定（リアルタイム更新）
		if (ImGui::DragFloat3("Scale", &editorScale_.x, 0.01f, 0.1f, 100.0f)) {
			CreateOrUpdatePreviewRock();
		}

		// 岩タイプ選択（リアルタイム更新）
		int currentRockTypeIndex = static_cast<int>(currentRockType_);
		if (ImGui::Combo("Rock Type", &currentRockTypeIndex, kRockTypeDisplayNames, kRockTypeCount)) {
			currentRockType_ = static_cast<RockType>(currentRockTypeIndex);
			CreateOrUpdatePreviewRock();
		}

		// 追加ボタン
		if (ImGui::Button("Add Rock", ImVec2(120, 30))) {
			FieldObjectData newObjectData;
			newObjectData.position = editorPosition_;
			newObjectData.rotation = editorRotation_;
			newObjectData.scale = editorScale_;
			newObjectData.rockType = currentRockType_;
			newObjectData.id = GetNextId();
			newObjectData.isSelected = false;

			AddObject(std::move(newObjectData));

			// 新規追加用データをリセット
			ResetNewRockData();
		}

		ImGui::Separator();

		// 配置済みRockリスト
		ImGui::Text("Placed Rocks: %zu", objectDataList_.size());

		if (ImGui::BeginChild("RockList", ImVec2(0, 300), true)) {
			for (size_t i = 0; i < objectDataList_.size(); ++i) {
				auto& objData = objectDataList_[i];

				ImGui::PushID(static_cast<int>(i));

				// 選択チェック
				bool isSelected = (selectedObjectId_ == objData.id);

				// より詳細なRock情報を表示
				std::string rockLabel = "Rock " + std::to_string(objData.id) +
					" [" + Rock::RockTypeToString(objData.rockType) + "]";

				if (ImGui::Selectable(rockLabel.c_str(), isSelected)) {
					selectedObjectId_ = isSelected ? -1 : objData.id;
					UpdateSelectionStates(selectedObjectId_);
				}

				// Rock情報を表示
				ImGui::Text("  Pos: (%.1f, %.1f, %.1f)",
					objData.position.x, objData.position.y, objData.position.z);
				ImGui::Text("  Rot: (%.1f, %.1f, %.1f) | Scale: (%.2f, %.2f, %.2f)",
					objData.rotation.x, objData.rotation.y, objData.rotation.z,
					objData.scale.x, objData.scale.y, objData.scale.z);

				// インライン編集（選択されている場合）
				if (selectedObjectId_ == objData.id) {
					DrawInlineEditor(objData, i);
				}

				// アクションボタン群
				DrawActionButtons(objData, i);

				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		// 選択中のRockの詳細編集
		if (IsValidSelectedIndex()) {
			DrawDetailedEditor();
		}
	}
#endif
}

bool FieldEditor::SaveToCSV(const std::string& fileName) {
	std::ofstream file(fileName);
	if (!file.is_open()) {
		return false;
	}

	// ヘッダー行
	file << "ID,RockType,PosX,PosY,PosZ,RotX,RotY,RotZ,ScaleX,ScaleY,ScaleZ\n";

	// データ行
	for (const auto& objData : objectDataList_) {
		file << objData.id << ","
			<< Rock::RockTypeToString(objData.rockType) << ","
			<< objData.position.x << "," << objData.position.y << "," << objData.position.z << ","
			<< objData.rotation.x << "," << objData.rotation.y << "," << objData.rotation.z << ","
			<< objData.scale.x << "," << objData.scale.y << "," << objData.scale.z << "\n";
	}

	file.close();
	return true;
}

bool FieldEditor::LoadFromCSV(const std::string& fileName) {
	std::ifstream file(fileName);
	if (!file.is_open()) {
		return false;
	}

	// 既存オブジェクトをクリア
	ClearAllObjects();

	std::string line;
	// ヘッダー行をスキップ
	if (!std::getline(file, line)) {
		return false;
	}

	// データ行を読み込み
	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string cell;
		std::vector<std::string> values;

		while (std::getline(ss, cell, ',')) {
			values.push_back(cell);
		}

		if (values.size() >= 11) {
			FieldObjectData objData;
			objData.id = std::stoi(values[0]);
			objData.rockType = Rock::StringToRockType(values[1]);
			objData.position = { std::stof(values[2]), std::stof(values[3]), std::stof(values[4]) };
			objData.rotation = { std::stof(values[5]), std::stof(values[6]), std::stof(values[7]) };
			objData.scale = { std::stof(values[8]), std::stof(values[9]), std::stof(values[10]) };
			objData.isSelected = false;

			AddObject(std::move(objData));

			// nextId_を更新
			if (objData.id >= nextId_) {
				nextId_ = objData.id + 1;
			}
		}
	}

	file.close();
	return true;
}

void FieldEditor::ClearAllObjects() {
	objectDataList_.clear();
	selectedObjectId_ = -1;
	nextId_ = 0;
}

void FieldEditor::AddObject(const FieldObjectData& objectData) {
	// ムーブコンストラクタを使用してデータを追加
	FieldObjectData newData;
	newData.position = objectData.position;
	newData.rotation = objectData.rotation;
	newData.scale = objectData.scale;
	newData.rockType = objectData.rockType;
	newData.id = objectData.id;
	newData.isSelected = false;

	// 実際のRockオブジェクトを作成
	newData.actualRock = std::make_unique<Rock>();
	newData.actualRock->Initialize(directXCommon_, newData.rockType,
		newData.position, newData.rotation, newData.scale);

	// 色を設定
	UpdateRockColor(newData.actualRock.get(), newData.rockType, false, false);

	objectDataList_.push_back(std::move(newData));
}

void FieldEditor::RemoveObject(int id) {
	auto it = std::find_if(objectDataList_.begin(), objectDataList_.end(),
		[id](const FieldObjectData& objData) { return objData.id == id; });

	if (it != objectDataList_.end()) {
		objectDataList_.erase(it);
	}
}

int FieldEditor::GetNextId() {
	return nextId_++;
}

void FieldEditor::CreateOrUpdatePreviewRock() {
	if (!directXCommon_) return;

	// プレビューRockを作成または更新
	if (!previewRock_) {
		previewRock_ = std::make_unique<Rock>();
	}

	previewRock_->Initialize(directXCommon_, currentRockType_,
		editorPosition_, editorRotation_, editorScale_);

	// プレビュー用の色を設定（白っぽく半透明）
	UpdateRockColor(previewRock_.get(), currentRockType_, false, true);
}

void FieldEditor::UpdateRockColor(Rock* rock, RockType rockType, bool isSelected, bool isPreview) {
	if (!rock) return;

	Vector4 color;

	if (isPreview) {
		// プレビュー用は白っぽく半透明
		color = { 1.0f, 1.0f, 1.0f, kPreviewAlpha };
	} else {
		// 岩タイプに応じた基本色
		color = GetRockTypeBaseColor(rockType);

		// 選択されている場合は明度を上げる
		if (isSelected) {
			color.x = std::min(1.0f, color.x + kSelectedColorBoost);
			color.y = std::min(1.0f, color.y + kSelectedColorBoost);
			color.z = std::min(1.0f, color.z + kSelectedColorBoost);
			color.w = kSelectedAlpha;
		}
	}

	rock->SetColor(color);
}

Vector4 FieldEditor::GetRockTypeBaseColor(RockType rockType) {
	switch (rockType) {
	case RockType::Rock1:
		return { 1.0f, kColorBrightness, kColorBrightness, kNormalAlpha }; // 赤系
	case RockType::Rock2:
		return { kColorBrightness, 1.0f, kColorBrightness, kNormalAlpha }; // 緑系
	case RockType::Rock3:
		return { kColorBrightness, kColorBrightness, 1.0f, kNormalAlpha }; // 青系
	default:
		return { 0.7f, 0.7f, 0.7f, kNormalAlpha }; // グレー
	}
}

void FieldEditor::ResetNewRockData() {
	editorPosition_ = { 0.0f, 0.0f, 0.0f };
	editorRotation_ = { 0.0f, 0.0f, 0.0f };
	editorScale_ = { 1.0f, 1.0f, 1.0f };
	currentRockType_ = RockType::Rock1;
	CreateOrUpdatePreviewRock();
}

void FieldEditor::UpdateSelectionStates(int selectedId) {
	for (auto& objData : objectDataList_) {
		objData.isSelected = (selectedId >= 0 && objData.id == selectedId);
		UpdateRockColor(objData.actualRock.get(), objData.rockType, objData.isSelected, false);
	}
}

bool FieldEditor::IsValidSelectedIndex() const {
	if (selectedObjectId_ < 0) return false;

	return std::any_of(objectDataList_.begin(), objectDataList_.end(),
		[this](const FieldObjectData& objData) { return objData.id == selectedObjectId_; });
}

void FieldEditor::DrawInlineEditor(FieldObjectData& objData, size_t index) {
#ifdef _DEBUG
	ImGui::Separator();
	ImGui::Text("Quick Edit:");

	// 座標の直接編集
	if (ImGui::DragFloat3(("Pos##" + std::to_string(index)).c_str(),
		&objData.position.x, 0.1f)) {
		if (objData.actualRock) {
			objData.actualRock->SetPosition(objData.position);
		}
	}

	// 回転の直接編集
	if (ImGui::DragFloat3(("Rot##" + std::to_string(index)).c_str(),
		&objData.rotation.x, 0.1f)) {
		if (objData.actualRock) {
			objData.actualRock->SetRotation(objData.rotation);
		}
	}

	// スケールの直接編集
	if (ImGui::DragFloat3(("Scale##" + std::to_string(index)).c_str(),
		&objData.scale.x, 0.01f, 0.1f, 100.0f)) {
		if (objData.actualRock) {
			objData.actualRock->SetScale(objData.scale);
		}
	}
#endif
}

void FieldEditor::DrawActionButtons(FieldObjectData& objData, size_t index) {
#ifdef _DEBUG
	// カメラ位置に移動ボタン
	if (ImGui::Button(("Set Cam Pos##" + std::to_string(index)).c_str())) {
		if (cameraController_) {
			objData.position = cameraController_->GetPosition();
			if (objData.actualRock) {
				objData.actualRock->SetPosition(objData.position);
			}
		}
	}

	// 削除ボタン
	ImGui::SameLine();
	if (ImGui::Button(("Delete##" + std::to_string(index)).c_str())) {
		RemoveObject(objData.id);
		if (selectedObjectId_ == objData.id) {
			selectedObjectId_ = -1;
		}
		return; // イテレータが無効になるので早期リターン
	}
#endif
}

void FieldEditor::DrawDetailedEditor() {
#ifdef _DEBUG
	ImGui::Separator();
	ImGui::Text("Edit Selected Rock:");

	// 選択されたオブジェクトを検索
	auto it = std::find_if(objectDataList_.begin(), objectDataList_.end(),
		[this](const FieldObjectData& objData) { return objData.id == selectedObjectId_; });

	if (it == objectDataList_.end()) return;

	auto& selectedData = *it;

	// 座標編集
	if (ImGui::DragFloat3("Edit Position", &selectedData.position.x, 0.1f)) {
		if (selectedData.actualRock) {
			selectedData.actualRock->SetPosition(selectedData.position);
		}
	}

	// 回転編集
	if (ImGui::DragFloat3("Edit Rotation", &selectedData.rotation.x, 0.1f)) {
		if (selectedData.actualRock) {
			selectedData.actualRock->SetRotation(selectedData.rotation);
		}
	}

	// スケール編集
	if (ImGui::DragFloat3("Edit Scale", &selectedData.scale.x, 0.01f, 0.1f, 100.0f)) {
		if (selectedData.actualRock) {
			selectedData.actualRock->SetScale(selectedData.scale);
		}
	}

	// 岩タイプ編集
	int editRockType = static_cast<int>(selectedData.rockType);
	if (ImGui::Combo("Edit Rock Type", &editRockType, kRockTypeDisplayNames, kRockTypeCount)) {
		selectedData.rockType = static_cast<RockType>(editRockType);
		if (selectedData.actualRock) {
			selectedData.actualRock->SetRockType(selectedData.rockType);
			UpdateRockColor(selectedData.actualRock.get(), selectedData.rockType, selectedData.isSelected, false);
		}
	}
#endif
}