#define NOMINMAX
#include "EnemyPlacementEditor.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "BaseSystem/Logger/Logger.h"
#include "EnemyPopCommand/EnemyPopCommand.h"
#include "CameraController/CameraController.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace {
	// エディタ定数
	constexpr float kNormalEnemyScale = 3.0f;
	constexpr float kRushingFishScale = 2.0f;
	constexpr float kShootingFishScale = 2.5f;

	constexpr float kNormalEnemyRadius = 3.0f;
	constexpr float kRushingFishRadius = 2.0f;
	constexpr float kShootingFishRadius = 2.5f;

	constexpr int kMaxWaitTime = 600;
	constexpr float kColorBrightness = 0.3f;
	constexpr float kSelectedColorBoost = 0.3f;
	constexpr float kNormalAlpha = 0.7f;
	constexpr float kSelectedAlpha = 0.9f;
	constexpr float kNewEnemyAlpha = 0.5f;

	// 敵タイプ表示名の配列
	const char* kEnemyTypeDisplayNames[] = {
		"Normal",
		"RushingFish",
		"ShootingFish"
	};
	constexpr int kEnemyTypeCount = 3;

	// パターン表示名の配列
	const char* kPatternDisplayNames[] = {
		"Straight",
		"LeaveLeft",
		"LeaveRight",
		"Homing",
		"Shooting"
	};
	constexpr int kPatternCount = 5;

	// 敵タイプごとのスケールを取得
	float GetEnemyTypeScale(EnemyType enemyType) {
		switch (enemyType) {
		case EnemyType::Normal: return kNormalEnemyScale;
		case EnemyType::RushingFish: return kRushingFishScale;
		case EnemyType::ShootingFish: return kShootingFishScale;
		default: return kNormalEnemyScale;
		}
	}

	// 敵タイプごとの半径を取得
	float GetEnemyTypeRadius(EnemyType enemyType) {
		switch (enemyType) {
		case EnemyType::Normal: return kNormalEnemyRadius;
		case EnemyType::RushingFish: return kRushingFishRadius;
		case EnemyType::ShootingFish: return kShootingFishRadius;
		default: return kNormalEnemyRadius;
		}
	}

	// 敵タイプごとの基本色を取得
	Vector4 GetEnemyTypeBaseColor(EnemyType enemyType) {
		switch (enemyType) {
		case EnemyType::Normal:
			return { 1.0f, kColorBrightness, kColorBrightness, kNormalAlpha }; // 赤系
		case EnemyType::RushingFish:
			return { kColorBrightness, kColorBrightness, 1.0f, kNormalAlpha }; // 青系
		case EnemyType::ShootingFish:
			return { kColorBrightness, 1.0f, kColorBrightness, kNormalAlpha }; // 緑系
		default:
			return { 0.5f, 0.5f, 0.5f, kNormalAlpha }; // グレー
		}
	}
}

EnemyPlacementEditor::EnemyPlacementEditor()
	: directXCommon_(nullptr)
	, cameraController_(nullptr)
	, isEditorEnabled_(false)
	, showPreviewModels_(true)
	, showCSVEnemies_(true)
	, selectedIndex_(-1)
	, csvFilePath_("resources/CSV_Data/Enemy_Pop/enemyPop.csv")
{
}

EnemyPlacementEditor::~EnemyPlacementEditor() {
}

void EnemyPlacementEditor::Initialize(DirectXCommon* dxCommon, CameraController* cameraController) {
	directXCommon_ = dxCommon;
	cameraController_ = cameraController;

	// 新規追加用データの初期化
	newEnemyData_ = EnemyPlacementData();
	// 初期プレビューモデルを作成
	if (directXCommon_) {
		CreatePreviewModel(newEnemyData_, true); // 新規追加用として作成
	}
}

void EnemyPlacementEditor::Update(const Matrix4x4& viewProjectionMatrix) {
	// エディタが無効なら早期リターン
	if (!isEditorEnabled_) {
		return;
	}

	// プレビューモデルの更新
	if (showPreviewModels_) {
		for (auto& placement : enemyPlacements_) {
			if (placement.previewModel) {
				placement.previewModel->Update(viewProjectionMatrix);
			}
		}
	}

	// 新規追加用プレビューモデルの更新
	if (newEnemyData_.previewModel) {
		newEnemyData_.previewModel->Update(viewProjectionMatrix);
	}
}

void EnemyPlacementEditor::Draw(const Light& directionalLight) {
	// エディタが無効なら早期リターン
	if (!isEditorEnabled_) {
		return;
	}

	// プレビューモデルの描画
	if (showPreviewModels_) {
		for (auto& placement : enemyPlacements_) {
			if (placement.previewModel) {
				placement.previewModel->Draw(directionalLight);
			}
		}
	}

	// 新規追加用プレビューモデルの描画
	if (newEnemyData_.previewModel) {
		newEnemyData_.previewModel->Draw(directionalLight);
	}
}

void EnemyPlacementEditor::ImGui() {
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Enemy Placement Editor")) {
		// エディタのon/off
		ImGui::Checkbox("Enable Editor", &isEditorEnabled_);

		if (!isEditorEnabled_) {
			return;
		}

		ImGui::Separator();

		// 表示設定
		ImGui::Text("Display Settings:");
		ImGui::Checkbox("Show Preview Models", &showPreviewModels_);
		ImGui::Checkbox("Show CSV Enemies", &showCSVEnemies_);

		ImGui::Separator();

		// ファイル操作
		ImGui::Text("File Operations:");
		if (ImGui::Button("Save to CSV")) {
			if (SaveToCSV(csvFilePath_)) {
				Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: CSV saved successfully\n");
			} else {
				Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: Failed to save CSV\n");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Load from CSV")) {
			if (LoadFromCSV(csvFilePath_)) {
				Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: CSV loaded successfully\n");
			} else {
				Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: Failed to load CSV\n");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear All")) {
			enemyPlacements_.clear();
			selectedIndex_ = -1;
		}

		ImGui::Separator();

		// 新規敵追加
		ImGui::Text("Add New Enemy:");

		// カメラ位置取得ボタン
		if (ImGui::Button("Get Camera Position")) {
			if (cameraController_) {
				// カメラの現在位置を直接取得
				newEnemyData_.position = cameraController_->GetPosition();

				// プレビューモデルを更新
				if (newEnemyData_.previewModel) {
					newEnemyData_.previewModel->SetPosition(newEnemyData_.position);
				} else {
					CreatePreviewModel(newEnemyData_, true);
				}
			}
		}
		ImGui::SameLine();
		// リセットボタン
		if (ImGui::Button("Reset Position")) {
			newEnemyData_.position = { 0.0f, 0.0f, 100.0f };
			if (newEnemyData_.previewModel) {
				newEnemyData_.previewModel->SetPosition(newEnemyData_.position);
			} else {
				CreatePreviewModel(newEnemyData_, true);
			}
		}

		// 座標設定
		if (ImGui::DragFloat3("Position", &newEnemyData_.position.x, 0.5f)) {
			if (newEnemyData_.previewModel) {
				newEnemyData_.previewModel->SetPosition(newEnemyData_.position);
			} else {
				CreatePreviewModel(newEnemyData_, true);
			}
		}

		// 敵タイプ選択
		int currentEnemyType = static_cast<int>(newEnemyData_.enemyType);
		if (ImGui::Combo("Enemy Type", &currentEnemyType, kEnemyTypeDisplayNames, kEnemyTypeCount)) {
			newEnemyData_.enemyType = static_cast<EnemyType>(currentEnemyType);
			// 新規追加用は白色を維持
			if (newEnemyData_.previewModel) {
				Vector4 color = { 1.0f, 1.0f, 1.0f, kNewEnemyAlpha };
				newEnemyData_.previewModel->SetColor(color);
			}
		}

		// パターン選択
		int currentPattern = static_cast<int>(newEnemyData_.pattern);
		if (ImGui::Combo("Pattern", &currentPattern, kPatternDisplayNames, kPatternCount)) {
			newEnemyData_.pattern = static_cast<EnemyPattern>(currentPattern);
		}

		// 待機時間
		ImGui::DragInt("Wait Time", &newEnemyData_.waitTime, 1, 0, kMaxWaitTime);

		if (ImGui::Button("Add Enemy")) {
			EnemyPlacementData newPlacement;
			newPlacement.position = newEnemyData_.position;
			newPlacement.enemyType = newEnemyData_.enemyType;
			newPlacement.pattern = newEnemyData_.pattern;
			newPlacement.waitTime = newEnemyData_.waitTime;
			newPlacement.isSelected = false;

			CreatePreviewModel(newPlacement, false);
			enemyPlacements_.push_back(std::move(newPlacement));

			// 新規追加用データをリセット
			ResetNewEnemyData();
		}

		ImGui::Separator();

		// 配置済み敵リスト
		ImGui::Text("Placed Enemies: %zu", enemyPlacements_.size());

		if (ImGui::BeginChild("EnemyList", ImVec2(0, 300), true)) {
			for (size_t i = 0; i < enemyPlacements_.size(); ++i) {
				auto& placement = enemyPlacements_[i];

				ImGui::PushID(static_cast<int>(i));

				// 選択チェック
				bool isSelected = (selectedIndex_ == static_cast<int>(i));

				// より詳細な敵情報を表示
				std::string enemyLabel = "Enemy " + std::to_string(i) +
					" [" + EnemyPopCommand::EnemyTypeToString(placement.enemyType) + "]";

				if (ImGui::Selectable(enemyLabel.c_str(), isSelected)) {
					selectedIndex_ = isSelected ? -1 : static_cast<int>(i);
					// 選択状態を更新
					UpdateSelectionStates(selectedIndex_);
				}

				// 敵情報を2行で表示
				ImGui::Text("  Pos: (%.1f, %.1f, %.1f) | Pattern: %s",
					placement.position.x, placement.position.y, placement.position.z,
					EnemyPopCommand::EnemyPatternToString(placement.pattern).c_str());
				ImGui::Text("  Wait: %d frames", placement.waitTime);

				// インライン編集（選択されている場合）
				if (selectedIndex_ == static_cast<int>(i)) {
					DrawInlineEditor(placement, i);
				}

				// アクションボタン群
				DrawActionButtons(placement, i);

				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		// 選択中の敵の詳細編集
		if (IsValidSelectedIndex()) {
			DrawDetailedEditor();
		}
	}
#endif
}

void EnemyPlacementEditor::LoadFromEnemyPopCommand(EnemyPopCommand* enemyPopCommand) {
	if (!enemyPopCommand) return;

	// 現在のデータをクリア
	enemyPlacements_.clear();

	// EnemyPopCommandから読み込み（CSVファイルを再解析）
	LoadFromCSV(csvFilePath_);
}

bool EnemyPlacementEditor::SaveToCSV(const std::string& filePath) {
	// EditorEnemyInfo のベクターを作成
	std::vector<EditorEnemyInfo> enemyInfos;
	enemyInfos.reserve(enemyPlacements_.size());

	for (const auto& placement : enemyPlacements_) {
		EditorEnemyInfo info;
		info.position = placement.position;
		info.enemyType = placement.enemyType;
		info.pattern = placement.pattern;
		info.waitTime = placement.waitTime;
		enemyInfos.push_back(info);
	}

	// EnemyPopCommand の static メソッドを使用して保存
	return EnemyPopCommand::SaveEnemyInfoToCSV(filePath, enemyInfos);
}

bool EnemyPlacementEditor::LoadFromCSV(const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: Failed to open file for reading: " + filePath + "\n");
		return false;
	}

	// 現在のデータをクリア
	enemyPlacements_.clear();

	std::string line;
	int currentWaitTime = 0; // 現在の待機時間

	while (std::getline(file, line)) {
		// 空行やコメント行をスキップ
		if (line.empty() || line.find("//") == 0) {
			continue;
		}

		std::istringstream lineStream(line);
		std::string command;
		std::getline(lineStream, command, ',');

		if (command == "WAIT") {
			// 待機コマンド
			std::string waitTimeStr;
			std::getline(lineStream, waitTimeStr, ',');
			currentWaitTime = std::atoi(waitTimeStr.c_str());
		} else if (command == "POP") {
			// 敵生成コマンド
			EnemyPlacementData placement;
			placement.waitTime = currentWaitTime;
			currentWaitTime = 0; // リセット

			// 座標読み込み
			std::string xStr, yStr, zStr, typeStr, patternStr;
			std::getline(lineStream, xStr, ',');
			std::getline(lineStream, yStr, ',');
			std::getline(lineStream, zStr, ',');
			std::getline(lineStream, typeStr, ',');
			std::getline(lineStream, patternStr, ',');

			placement.position.x = static_cast<float>(std::atof(xStr.c_str()));
			placement.position.y = static_cast<float>(std::atof(yStr.c_str()));
			placement.position.z = static_cast<float>(std::atof(zStr.c_str()));
			placement.enemyType = EnemyPopCommand::StringToEnemyType(typeStr);
			placement.pattern = EnemyPopCommand::StringToEnemyPattern(patternStr);

			// プレビューモデルを作成
			CreatePreviewModel(placement, false);

			enemyPlacements_.push_back(std::move(placement));
		}
	}

	file.close();
	return true;
}

void EnemyPlacementEditor::CreatePreviewModel(EnemyPlacementData& placement, bool isNewEnemy) {
	if (!directXCommon_) return;

	// 球体モデルを作成
	placement.previewModel = std::make_unique<Sphere>();
	placement.previewModel->Initialize(directXCommon_, "sphere", "uvChecker");
	placement.previewModel->SetPosition(placement.position);

	// 敵タイプに応じてサイズを調整
	float scale = GetEnemyTypeScale(placement.enemyType);

	Vector3Transform transform{
		{scale, scale, scale},
		{0.0f, 0.0f, 0.0f},
		placement.position
	};
	placement.previewModel->SetTransform(transform);

	// 色を設定
	if (isNewEnemy) {
		// 新規追加用は白っぽく表示
		Vector4 color = { 1.0f, 1.0f, 1.0f, kNewEnemyAlpha };
		placement.previewModel->SetColor(color);
	} else {
		UpdatePreviewModelColor(placement);
	}
}

void EnemyPlacementEditor::UpdatePreviewModelColor(EnemyPlacementData& placement) {
	if (!placement.previewModel) return;

	// 敵タイプに応じて色を変更
	Vector4 color = GetEnemyTypeBaseColor(placement.enemyType);

	// 選択されている場合は明度を上げる
	if (placement.isSelected) {
		color.x = std::min(1.0f, color.x + kSelectedColorBoost);
		color.y = std::min(1.0f, color.y + kSelectedColorBoost);
		color.z = std::min(1.0f, color.z + kSelectedColorBoost);
		color.w = kSelectedAlpha; // 不透明度も上げる
	}

	placement.previewModel->SetColor(color);
}

void EnemyPlacementEditor::ResetNewEnemyData() {
	newEnemyData_.position = { 0.0f, 0.0f, 100.0f };
	newEnemyData_.enemyType = EnemyType::Normal;
	newEnemyData_.pattern = EnemyPattern::Straight;
	newEnemyData_.waitTime = 0;
	// プレビューモデルをリセット
	newEnemyData_.previewModel.reset();
}

void EnemyPlacementEditor::UpdateSelectionStates(int selectedIndex) {
	for (size_t i = 0; i < enemyPlacements_.size(); ++i) {
		auto& placement = enemyPlacements_[i];
		placement.isSelected = (selectedIndex >= 0 && i == static_cast<size_t>(selectedIndex));
		UpdatePreviewModelColor(placement);
	}
}

bool EnemyPlacementEditor::IsValidSelectedIndex() const {
	return selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(enemyPlacements_.size());
}

void EnemyPlacementEditor::DrawInlineEditor(EnemyPlacementData& placement, size_t index) {
	ImGui::Separator();
	ImGui::Text("Quick Edit:");

	// 座標の直接編集
	if (ImGui::DragFloat3(("Pos##" + std::to_string(index)).c_str(),
		&placement.position.x, 0.5f)) {
		if (placement.previewModel) {
			placement.previewModel->SetPosition(placement.position);
		}
	}

	// 待機時間の直接編集
	ImGui::DragInt(("Wait##" + std::to_string(index)).c_str(),
		&placement.waitTime, 1, 0, kMaxWaitTime);
}

void EnemyPlacementEditor::DrawActionButtons(EnemyPlacementData& placement, size_t index) {
	// 削除ボタン
	ImGui::SameLine();
	if (ImGui::Button(("Delete##" + std::to_string(index)).c_str())) {
		enemyPlacements_.erase(enemyPlacements_.begin() + index);
		if (selectedIndex_ >= static_cast<int>(index)) {
			selectedIndex_--;
		}
		return; // イテレータが無効になるので早期リターン
	}

	// カメラ位置に移動ボタン
	ImGui::SameLine();
	if (ImGui::Button(("Set Cam Pos##" + std::to_string(index)).c_str())) {
		if (cameraController_) {
			// カメラの現在位置を直接取得
			placement.position = cameraController_->GetPosition();

			// プレビューモデルを更新
			if (placement.previewModel) {
				placement.previewModel->SetPosition(placement.position);
			}
		}
	}
}

void EnemyPlacementEditor::DrawDetailedEditor() {
	ImGui::Separator();
	ImGui::Text("Edit Selected Enemy:");

	auto& selectedPlacement = enemyPlacements_[selectedIndex_];

	// 座標編集
	if (ImGui::DragFloat3("Edit Position", &selectedPlacement.position.x, 0.5f)) {
		if (selectedPlacement.previewModel) {
			selectedPlacement.previewModel->SetPosition(selectedPlacement.position);
		}
	}

	// 敵タイプ編集
	int editEnemyType = static_cast<int>(selectedPlacement.enemyType);
	if (ImGui::Combo("Edit Enemy Type", &editEnemyType, kEnemyTypeDisplayNames, kEnemyTypeCount)) {
		selectedPlacement.enemyType = static_cast<EnemyType>(editEnemyType);
		UpdatePreviewModelColor(selectedPlacement);
	}

	// パターン編集
	int editPattern = static_cast<int>(selectedPlacement.pattern);
	if (ImGui::Combo("Edit Pattern", &editPattern, kPatternDisplayNames, kPatternCount)) {
		selectedPlacement.pattern = static_cast<EnemyPattern>(editPattern);
	}

	// 待機時間編集
	ImGui::DragInt("Edit Wait Time", &selectedPlacement.waitTime, 1, 0, kMaxWaitTime);
}