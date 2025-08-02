#define NOMINMAX
#include "EnemyPlacementEditor.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "BaseSystem/Logger/Logger.h"
#include "EnemyPopCommand/EnemyPopCommand.h"
#include "CameraController/CameraController.h"
#include <fstream>
#include <sstream>

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
				Vector3 position = cameraController_->GetPosition();

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
		const char* enemyTypeItems[] = { "Normal", "RushingFish" };
		int currentEnemyType = static_cast<int>(newEnemyData_.enemyType);
		if (ImGui::Combo("Enemy Type", &currentEnemyType, enemyTypeItems, IM_ARRAYSIZE(enemyTypeItems))) {
			newEnemyData_.enemyType = static_cast<EnemyType>(currentEnemyType);
			// 新規追加用は白色を維持
			if (newEnemyData_.previewModel) {
				Vector4 color = { 1.0f, 1.0f, 1.0f, 0.5f };
				newEnemyData_.previewModel->SetColor(color);
			}
		}

		// パターン選択
		const char* patternItems[] = { "Straight", "LeaveLeft", "LeaveRight", "Homing" };
		int currentPattern = static_cast<int>(newEnemyData_.pattern);
		if (ImGui::Combo("Pattern", &currentPattern, patternItems, IM_ARRAYSIZE(patternItems))) {
			newEnemyData_.pattern = static_cast<EnemyPattern>(currentPattern);
		}

		// 待機時間
		ImGui::DragInt("Wait Time", &newEnemyData_.waitTime, 1, 0, 600);

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
			newEnemyData_.position = { 0.0f, 0.0f, 100.0f };
			newEnemyData_.enemyType = EnemyType::Normal;
			newEnemyData_.pattern = EnemyPattern::Straight;
			newEnemyData_.waitTime = 0;
			// プレビューモデルをリセット
			newEnemyData_.previewModel.reset();
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
					" [" + EnemyTypeToString(placement.enemyType) + "]";

				if (ImGui::Selectable(enemyLabel.c_str(), isSelected)) {
					selectedIndex_ = isSelected ? -1 : static_cast<int>(i);
					// 選択状態を更新
					for (auto& p : enemyPlacements_) {
						p.isSelected = false;
					}
					if (selectedIndex_ >= 0) {
						placement.isSelected = true;
						UpdatePreviewModelColor(placement);
					}
				}

				// 敵情報を2行で表示
				ImGui::Text("  Pos: (%.1f, %.1f, %.1f) | Pattern: %s",
					placement.position.x, placement.position.y, placement.position.z,
					EnemyPatternToString(placement.pattern).c_str());
				ImGui::Text("  Wait: %d frames", placement.waitTime);

				// インライン編集（選択されている場合）
				if (selectedIndex_ == static_cast<int>(i)) {
					ImGui::Separator();
					ImGui::Text("Quick Edit:");

					// 座標の直接編集
					if (ImGui::DragFloat3(("Pos##" + std::to_string(i)).c_str(),
						&placement.position.x, 0.5f)) {
						if (placement.previewModel) {
							placement.previewModel->SetPosition(placement.position);
						}
					}

					// 待機時間の直接編集
					ImGui::DragInt(("Wait##" + std::to_string(i)).c_str(),
						&placement.waitTime, 1, 0, 600);
				}

				// 削除ボタン
				ImGui::SameLine();
				if (ImGui::Button(("Delete##" + std::to_string(i)).c_str())) {
					enemyPlacements_.erase(enemyPlacements_.begin() + i);
					if (selectedIndex_ >= static_cast<int>(i)) {
						selectedIndex_--;
					}
					ImGui::PopID();
					break; // イテレータが無効になるのでbreak
				}

				// カメラ位置に移動ボタン
				ImGui::SameLine();
				if (ImGui::Button(("Set Cam Pos##" + std::to_string(i)).c_str())) {
					if (cameraController_) {
						// カメラの現在位置を直接取得
						placement.position = cameraController_->GetPosition();

						// プレビューモデルを更新
						if (placement.previewModel) {
							placement.previewModel->SetPosition(placement.position);
						}
					}
				}

				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		// 選択中の敵の詳細編集
		if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(enemyPlacements_.size())) {
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
			if (ImGui::Combo("Edit Enemy Type", &editEnemyType, enemyTypeItems, IM_ARRAYSIZE(enemyTypeItems))) {
				selectedPlacement.enemyType = static_cast<EnemyType>(editEnemyType);
				UpdatePreviewModelColor(selectedPlacement);
			}

			// パターン編集
			int editPattern = static_cast<int>(selectedPlacement.pattern);
			if (ImGui::Combo("Edit Pattern", &editPattern, patternItems, IM_ARRAYSIZE(patternItems))) {
				selectedPlacement.pattern = static_cast<EnemyPattern>(editPattern);
			}

			// 待機時間編集
			ImGui::DragInt("Edit Wait Time", &selectedPlacement.waitTime, 1, 0, 600);
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
	std::ofstream file(filePath);
	if (!file.is_open()) {
		Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: Failed to open file for writing: " + filePath + "\n");
		return false;
	}

	// ヘッダーコメントを書き込み
	file << "// Enemy Placement Data\n";
	file << "// Format: COMMAND,X,Y,Z,TYPE,PATTERN or WAIT,TIME\n";
	file << "\n";

	// 敵配置データを書き込み
	for (const auto& placement : enemyPlacements_) {
		// 待機コマンドを先に書き込み（0でない場合）
		if (placement.waitTime > 0) {
			file << "WAIT," << placement.waitTime << "\n";
		}

		// 敵生成コマンドを書き込み
		file << "POP,"
			<< placement.position.x << ","
			<< placement.position.y << ","
			<< placement.position.z << ","
			<< EnemyTypeToString(placement.enemyType) << ","
			<< EnemyPatternToString(placement.pattern) << "\n";
	}

	file.close();
	return true;
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
			placement.enemyType = StringToEnemyType(typeStr);
			placement.pattern = StringToEnemyPattern(patternStr);

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
	Vector3 scale;
	switch (placement.enemyType) {
	case EnemyType::Normal:
		scale = { 3.0f, 3.0f, 3.0f };
		break;
	case EnemyType::RushingFish:
		scale = { 2.0f, 2.0f, 2.0f };
		break;
	default:
		scale = { 3.0f, 3.0f, 3.0f };
		break;
	}

	Vector3Transform transform{
		scale,
		{0.0f, 0.0f, 0.0f},
		placement.position
	};
	placement.previewModel->SetTransform(transform);

	// 色を設定
	if (isNewEnemy) {
		// 新規追加用は白っぽく表示
		Vector4 color = { 1.0f, 1.0f, 1.0f, 0.5f };
		placement.previewModel->SetColor(color);
	} else {
		UpdatePreviewModelColor(placement);
	}
}

void EnemyPlacementEditor::UpdatePreviewModelColor(EnemyPlacementData& placement) {
	if (!placement.previewModel) return;

	// 敵タイプに応じて色を変更
	Vector4 color;
	switch (placement.enemyType) {
	case EnemyType::Normal:
		color = { 1.0f, 0.3f, 0.3f, 0.7f }; // 赤系
		break;
	case EnemyType::RushingFish:
		color = { 0.3f, 0.3f, 1.0f, 0.7f }; // 青系
		break;
	default:
		color = { 0.5f, 0.5f, 0.5f, 0.7f }; // グレー
		break;
	}

	// 選択されている場合は明度を上げる
	if (placement.isSelected) {
		color.x = std::min(1.0f, color.x + 0.3f);
		color.y = std::min(1.0f, color.y + 0.3f);
		color.z = std::min(1.0f, color.z + 0.3f);
		color.w = 0.9f; // 不透明度も上げる
	}

	placement.previewModel->SetColor(color);
}

std::string EnemyPlacementEditor::EnemyTypeToString(EnemyType type) {
	switch (type) {
	case EnemyType::Normal: return "Normal";
	case EnemyType::RushingFish: return "RushingFish";
	default: return "Normal";
	}
}

std::string EnemyPlacementEditor::EnemyPatternToString(EnemyPattern pattern) {
	switch (pattern) {
	case EnemyPattern::Straight: return "Straight";
	case EnemyPattern::LeaveLeft: return "LeaveLeft";
	case EnemyPattern::LeaveRight: return "LeaveRight";
	case EnemyPattern::Homing: return "Homing";
	default: return "Straight";
	}
}

EnemyType EnemyPlacementEditor::StringToEnemyType(const std::string& typeStr) {
	if (typeStr == "Normal" || typeStr == "0") {
		return EnemyType::Normal;
	} else if (typeStr == "RushingFish" || typeStr == "1") {
		return EnemyType::RushingFish;
	}
	return EnemyType::Normal;
}

EnemyPattern EnemyPlacementEditor::StringToEnemyPattern(const std::string& patternStr) {
	if (patternStr == "Straight" || patternStr == "0") {
		return EnemyPattern::Straight;
	} else if (patternStr == "LeaveLeft" || patternStr == "1") {
		return EnemyPattern::LeaveLeft;
	} else if (patternStr == "LeaveRight" || patternStr == "2") {
		return EnemyPattern::LeaveRight;
	} else if (patternStr == "Homing" || patternStr == "3") {
		return EnemyPattern::Homing;
	}
	return EnemyPattern::Straight;
}