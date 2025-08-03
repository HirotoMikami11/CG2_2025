#define NOMINMAX
#include "EnemyPlacementEditor.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "BaseSystem/Logger/Logger.h"
#include "EnemyPopCommand/EnemyPopCommand.h"
#include "CameraController/CameraController.h"
#include "Managers/ImGui/MyImGui.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <iomanip>

namespace {
	// エディタ定数
	constexpr float kNormalEnemyScale = 3.0f;
	constexpr float kRushingFishScale = 5.0f;
	constexpr float kShootingFishScale = 5.0f;

	constexpr float kNormalEnemyRadius = 3.0f;
	constexpr float kRushingFishRadius = 4.0f;
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
	, showActualModels_(true)
	, selectedIndex_(-1)
	, csvFilePath_("resources/CSV_Data/Enemy_Pop/enemyPop.csv")
	, temporaryFolderPath_("resources/CSV_Data/Enemy_Pop/temp/")
	, preserveNewEnemySettings_(true)
	, selectedPresetIndex_(0)
	, hasClipboardData_(false)
{
	// プリセットの初期化
	InitializePresets();
}

EnemyPlacementEditor::~EnemyPlacementEditor() {
}

void EnemyPlacementEditor::Initialize(DirectXCommon* dxCommon, CameraController* cameraController) {
	directXCommon_ = dxCommon;
	cameraController_ = cameraController;

	// 一時保存フォルダの作成
	std::filesystem::create_directories(temporaryFolderPath_);

	// 新規追加用データの初期化
	newEnemyData_ = EnemyPlacementData();
	// 初期プレビューモデルを作成
	if (directXCommon_) {
		CreatePreviewModel(newEnemyData_, true); // 新規追加用として作成
	}
}

void EnemyPlacementEditor::InitializePresets() {
	enemyPresets_.clear();

	// 各敵タイプのプリセットを定義
	enemyPresets_.emplace_back("Normal Enemy", EnemyType::Normal, EnemyPattern::Straight, 0,
		GetEnemyDefaultScale(EnemyType::Normal));
	enemyPresets_.emplace_back("Rushing Fish", EnemyType::RushingFish, EnemyPattern::Homing, 60,
		GetEnemyDefaultScale(EnemyType::RushingFish));
	enemyPresets_.emplace_back("Shooting Fish", EnemyType::ShootingFish, EnemyPattern::Shooting, 120,
		GetEnemyDefaultScale(EnemyType::ShootingFish));
}

void EnemyPlacementEditor::ApplyPreset(const EnemyPreset& preset) {
	newEnemyData_.enemyType = preset.enemyType;
	newEnemyData_.pattern = preset.pattern;
	newEnemyData_.waitTime = preset.waitTime;
	newEnemyData_.modelScale = preset.defaultScale;
	newEnemyData_.modelName = GetEnemyModelName(preset.enemyType);

	// プレビューモデルを更新
	UpdatePreviewModelType(newEnemyData_);

	// 新規追加用は白色を維持
	Vector4 color = { 1.0f, 1.0f, 1.0f, kNewEnemyAlpha };
	newEnemyData_.previewModel->SetColor(color);
}

std::string EnemyPlacementEditor::GetEnemyModelName(EnemyType enemyType) const {
	switch (enemyType) {
	case EnemyType::Normal: return "sphere";
	case EnemyType::RushingFish: return "rushFish";
	case EnemyType::ShootingFish: return "shootingFish";
	default: return "sphere";
	}
}

Vector3 EnemyPlacementEditor::GetEnemyDefaultScale(EnemyType enemyType) const {
	switch (enemyType) {
	case EnemyType::Normal: return { kNormalEnemyScale, kNormalEnemyScale, kNormalEnemyScale };
	case EnemyType::RushingFish: return { kRushingFishScale, kRushingFishScale, kRushingFishScale };
	case EnemyType::ShootingFish: return { kShootingFishScale, kShootingFishScale, kShootingFishScale };
	default: return { kNormalEnemyScale, kNormalEnemyScale, kNormalEnemyScale };
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

			// モデル関連の設定
			placement.modelName = GetEnemyModelName(placement.enemyType);
			placement.modelScale = GetEnemyDefaultScale(placement.enemyType);

			// プレビューモデルを作成
			CreatePreviewModel(placement, false);

			enemyPlacements_.push_back(std::move(placement));
		}
	}

	file.close();
	return true;
}

bool EnemyPlacementEditor::SaveTemporary() {
	// タイムスタンプ付きファイル名を生成
	std::string filename = GenerateTimestampedFilename();
	std::string fullPath = temporaryFolderPath_ + filename;

	bool result = SaveToCSV(fullPath);
	if (result) {
		Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: Temporary file saved: " + fullPath + "\n");
	}
	return result;
}

bool EnemyPlacementEditor::LoadTemporary() {
	// 一時保存ファイルのリストを取得
	auto fileList = GetTemporaryFileList();
	if (fileList.empty()) return false;

	// 最新のファイルを読み込み
	std::string latestFile = temporaryFolderPath_ + fileList.back();

	// 現在のすべての敵を削除
	enemyPlacements_.clear();
	selectedIndex_ = -1;

	bool result = LoadFromCSV(latestFile);
	if (result) {
		Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: Temporary file loaded: " + latestFile + "\n");
	}
	return result;
}

std::vector<std::string> EnemyPlacementEditor::GetTemporaryFileList() const {
	std::vector<std::string> files;

	try {
		for (const auto& entry : std::filesystem::directory_iterator(temporaryFolderPath_)) {
			if (entry.path().extension() == ".csv") {
				files.push_back(entry.path().filename().string());
			}
		}
		std::sort(files.begin(), files.end());
	} catch (const std::exception& e) {
		Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: Error reading temporary folder: " + std::string(e.what()) + "\n");
	}

	return files;
}

void EnemyPlacementEditor::CopySelectedEnemy() {
	if (!IsValidSelectedIndex()) return;

	// 選択された敵をクリップボードにコピー
	const auto& selected = enemyPlacements_[selectedIndex_];
	clipboardData_.position = selected.position;
	clipboardData_.enemyType = selected.enemyType;
	clipboardData_.pattern = selected.pattern;
	clipboardData_.waitTime = selected.waitTime;
	clipboardData_.modelName = selected.modelName;
	clipboardData_.modelScale = selected.modelScale;

	hasClipboardData_ = true;
	Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: Enemy copied to clipboard\n");
}

void EnemyPlacementEditor::PasteEnemyFromClipboard() {
	if (!hasClipboardData_) return;

	// クリップボードから新しい敵を配置
	EnemyPlacementData newPlacement;
	newPlacement.position = clipboardData_.position;
	newPlacement.enemyType = clipboardData_.enemyType;
	newPlacement.pattern = clipboardData_.pattern;
	newPlacement.waitTime = clipboardData_.waitTime;
	newPlacement.modelName = clipboardData_.modelName;
	newPlacement.modelScale = clipboardData_.modelScale;
	newPlacement.isSelected = false;

	CreatePreviewModel(newPlacement, false);
	enemyPlacements_.push_back(std::move(newPlacement));

	Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: Enemy pasted from clipboard\n");
}

std::string EnemyPlacementEditor::GenerateTimestampedFilename() const {

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };

	std::string timestamp = std::format("{:%Y%m%d_%H%M%S}", localTime);
	std::string filename = std::format("enemyPop_{}.csv", timestamp);

	return filename;
}

void EnemyPlacementEditor::CreatePreviewModel(EnemyPlacementData& placement, bool isNewEnemy) {
	if (!directXCommon_) return;

	// モデルの種類を決定
	if (showActualModels_ && placement.enemyType != EnemyType::Normal) {
		// 実際のモデルを使用
		placement.previewModel = std::make_unique<Model3D>();
		placement.previewModel->Initialize(directXCommon_, placement.modelName);
	} else {
		// 球体モデルを使用
		placement.previewModel = std::make_unique<Sphere>();
		placement.previewModel->Initialize(directXCommon_, "sphere", "uvChecker");
	}

	placement.previewModel->SetPosition(placement.position);

	// 敵タイプに応じてサイズを調整
	Vector3Transform transform{
		placement.modelScale,
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

void EnemyPlacementEditor::UpdatePreviewModelType(EnemyPlacementData& placement) {
	// モデル名とスケールを更新
	placement.modelName = GetEnemyModelName(placement.enemyType);
	placement.modelScale = GetEnemyDefaultScale(placement.enemyType);

	// プレビューモデルを再作成
	CreatePreviewModel(placement, false);
}

void EnemyPlacementEditor::UpdatePreviewModelColor(EnemyPlacementData& placement) {
	if (!placement.previewModel) return;

	Vector4 color;

	if (placement.isSelected) {
		// 選択されている場合は黒っぽい色
		color = GetSelectedColor();
	} else {
		// 敵タイプに応じて色を変更
		color = GetEnemyTypeBaseColor(placement.enemyType);
	}

	placement.previewModel->SetColor(color);
}

void EnemyPlacementEditor::ResetNewEnemyData() {
	if (!preserveNewEnemySettings_) {
		newEnemyData_.position = { 0.0f, 0.0f, 100.0f };
		newEnemyData_.enemyType = EnemyType::Normal;
		newEnemyData_.pattern = EnemyPattern::Straight;
		newEnemyData_.waitTime = 0;
		newEnemyData_.modelName = "sphere";
		newEnemyData_.modelScale = GetEnemyDefaultScale(EnemyType::Normal);
	}

	// プレビューモデルはリセット
	if (directXCommon_) {
		CreatePreviewModel(newEnemyData_, true);
	}
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

void EnemyPlacementEditor::ImGui() {
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Enemy Placement Editor")) {
		// エディタのon/off
		ImGui::Checkbox("Enable Editor", &isEditorEnabled_);

		if (!isEditorEnabled_) {
			return;
		}

		ImGui::Separator();

		// ファイル操作セクション
		DrawFileOperations();

		ImGui::Separator();

		// 表示設定
		ImGui::Text("Display Settings:");
		ImGui::Checkbox("Show Preview Models", &showPreviewModels_);
		ImGui::SameLine();
		ImGui::Checkbox("Show CSV Enemies", &showCSVEnemies_);
		ImGui::Checkbox("Show Actual Models", &showActualModels_);
		ImGui::Checkbox("Preserve New Enemy Settings", &preserveNewEnemySettings_);

		ImGui::Separator();

		// プリセット選択セクション
		DrawPresetSelection();

		ImGui::Separator();

		// 新規敵追加セクション
		DrawNewEnemySection();

		ImGui::Separator();

		// 敵リスト表示
		DrawEnemyList();

		// 選択中の敵の詳細編集
		if (IsValidSelectedIndex()) {
			DrawDetailedEditor();
		}
	}
#endif
}

void EnemyPlacementEditor::DrawFileOperations() {
#ifdef _DEBUG
	myImGui::CenterText("File Operations");
	ImGui::Spacing();

	// 基本ファイル操作
	if (ImGui::Button("Save to CSV")) {
		if (SaveToCSV(csvFilePath_)) {
			Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: CSV saved successfully\n");
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Load from CSV")) {
		if (LoadFromCSV(csvFilePath_)) {
			Logger::Log(Logger::GetStream(), "EnemyPlacementEditor: CSV loaded successfully\n");
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear All")) {
		enemyPlacements_.clear();
		selectedIndex_ = -1;
	}

	ImGui::Spacing();

	// 一時保存・再生機能
	myImGui::CenterText("Temporary Save/Load");
	if (ImGui::Button("Save Temporary")) {
		SaveTemporary();
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Temporary")) {
		// 全ての敵を削除してから読み込み
		enemyPlacements_.clear();
		selectedIndex_ = -1;
		LoadTemporary();
	}

	// 一時保存ファイルのリスト表示
	auto tempFiles = GetTemporaryFileList();
	if (!tempFiles.empty()) {
		ImGui::Text("Temporary files: %zu", tempFiles.size());
		if (ImGui::BeginCombo("Temp Files", tempFiles.empty() ? "None" : tempFiles.back().c_str())) {
			for (const auto& file : tempFiles) {
				if (ImGui::Selectable(file.c_str())) {
					enemyPlacements_.clear();
					selectedIndex_ = -1;
					LoadFromCSV(temporaryFolderPath_ + file);
				}
			}
			ImGui::EndCombo();
		}
	}

	ImGui::Spacing();

	// コピー・ペースト機能
	if (IsValidSelectedIndex()) {
		if (ImGui::Button("Copy Selected")) {
			CopySelectedEnemy();
		}
		ImGui::SameLine();
	}
	if (hasClipboardData_) {
		if (ImGui::Button("Paste from Clipboard")) {
			PasteEnemyFromClipboard();
		}
		ImGui::SameLine();
		ImGui::Text("(Clipboard: %s)", EnemyPopCommand::EnemyTypeToString(clipboardData_.enemyType).c_str());
	}
#endif
}

void EnemyPlacementEditor::DrawPresetSelection() {
#ifdef _DEBUG
	myImGui::CenterText("Enemy Presets");
	ImGui::Spacing();

	// プリセット選択
	std::vector<const char*> presetNames;
	for (const auto& preset : enemyPresets_) {
		presetNames.push_back(preset.name.c_str());
	}

	if (ImGui::Combo("Preset", &selectedPresetIndex_, presetNames.data(), static_cast<int>(presetNames.size()))) {
		if (selectedPresetIndex_ >= 0 && selectedPresetIndex_ < static_cast<int>(enemyPresets_.size())) {
			ApplyPreset(enemyPresets_[selectedPresetIndex_]);
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Apply Preset")) {
		if (selectedPresetIndex_ >= 0 && selectedPresetIndex_ < static_cast<int>(enemyPresets_.size())) {
			ApplyPreset(enemyPresets_[selectedPresetIndex_]);
		}
	}
#endif
}

void EnemyPlacementEditor::DrawNewEnemySection() {
#ifdef _DEBUG
	myImGui::CenterText("Add New Enemy");
	ImGui::Spacing();

	// カメラ位置取得とリセット
	if (ImGui::Button("Get Camera Position")) {
		if (cameraController_) {
			newEnemyData_.position = cameraController_->GetPosition();
			if (newEnemyData_.previewModel) {
				newEnemyData_.previewModel->SetPosition(newEnemyData_.position);
			} else {
				CreatePreviewModel(newEnemyData_, true);
			}
		}
	}
	ImGui::SameLine();
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
		UpdatePreviewModelType(newEnemyData_);
		// 新規追加用は白色を維持
		Vector4 color = { 1.0f, 1.0f, 1.0f, kNewEnemyAlpha };
		newEnemyData_.previewModel->SetColor(color);
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
		newPlacement.modelName = newEnemyData_.modelName;
		newPlacement.modelScale = newEnemyData_.modelScale;
		newPlacement.isSelected = false;

		CreatePreviewModel(newPlacement, false);
		enemyPlacements_.push_back(std::move(newPlacement));

		// 設定保持オプションが無効な場合のみリセット
		if (!preserveNewEnemySettings_) {
			ResetNewEnemyData();
		}
	}
#endif
}

void EnemyPlacementEditor::DrawEnemyList() {
#ifdef _DEBUG
	myImGui::CenterText("Placed Enemies");
	ImGui::Text("Count: %zu", enemyPlacements_.size());

	if (ImGui::BeginChild("EnemyList", ImVec2(0, 300), true)) {
		for (size_t i = 0; i < enemyPlacements_.size(); ++i) {
			auto& placement = enemyPlacements_[i];
			ImGui::PushID(static_cast<int>(i));

			bool isSelected = (selectedIndex_ == static_cast<int>(i));
			std::string enemyLabel = "Enemy " + std::to_string(i) +
				" [" + EnemyPopCommand::EnemyTypeToString(placement.enemyType) + "]";

			if (ImGui::Selectable(enemyLabel.c_str(), isSelected)) {
				selectedIndex_ = isSelected ? -1 : static_cast<int>(i);
				UpdateSelectionStates(selectedIndex_);
			}

			// 敵情報を2行で表示
			ImGui::Text("  Pos: (%.1f, %.1f, %.1f) | Pattern: %s",
				placement.position.x, placement.position.y, placement.position.z,
				EnemyPopCommand::EnemyPatternToString(placement.pattern).c_str());
			ImGui::Text("  Wait: %d frames | Model: %s",
				placement.waitTime, placement.modelName.c_str());

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
#endif
}

void EnemyPlacementEditor::DrawInlineEditor(EnemyPlacementData& placement, size_t index) {
#ifdef _DEBUG
	ImGui::Separator();
	ImGui::Text("Quick Edit:");

	// 座標の直接編集
	if (ImGui::DragFloat3(("Pos##" + std::to_string(index)).c_str(),
		&placement.position.x, 0.5f)) {
		if (placement.previewModel) {
			placement.previewModel->SetPosition(placement.position);
		}
	}

	// 敵タイプの直接編集
	int currentType = static_cast<int>(placement.enemyType);
	if (ImGui::Combo(("Type##" + std::to_string(index)).c_str(), &currentType, kEnemyTypeDisplayNames, kEnemyTypeCount)) {
		placement.enemyType = static_cast<EnemyType>(currentType);
		UpdatePreviewModelType(placement);
		UpdatePreviewModelColor(placement);
	}

	// パターンの直接編集
	int currentPattern = static_cast<int>(placement.pattern);
	if (ImGui::Combo(("Pattern##" + std::to_string(index)).c_str(), &currentPattern, kPatternDisplayNames, kPatternCount)) {
		placement.pattern = static_cast<EnemyPattern>(currentPattern);
	}

	// 待機時間の直接編集
	ImGui::DragInt(("Wait##" + std::to_string(index)).c_str(),
		&placement.waitTime, 1, 0, kMaxWaitTime);
#endif
}

void EnemyPlacementEditor::DrawActionButtons(EnemyPlacementData& placement, size_t index) {
#ifdef _DEBUG
	// 削除ボタン
	if (ImGui::Button(("Delete##" + std::to_string(index)).c_str())) {
		enemyPlacements_.erase(enemyPlacements_.begin() + index);
		if (selectedIndex_ >= static_cast<int>(index)) {
			selectedIndex_--;
		}
		return; // イテレータが無効になるので早期リターン
	}

	// コピーボタン
	ImGui::SameLine();
	if (ImGui::Button(("Copy##" + std::to_string(index)).c_str())) {
		selectedIndex_ = static_cast<int>(index);
		UpdateSelectionStates(selectedIndex_);
		CopySelectedEnemy();
	}

	// カメラ位置に移動ボタン
	ImGui::SameLine();
	if (ImGui::Button(("Set Cam Pos##" + std::to_string(index)).c_str())) {
		if (cameraController_) {
			placement.position = cameraController_->GetPosition();
			if (placement.previewModel) {
				placement.previewModel->SetPosition(placement.position);
			}
		}
	}
#endif
}

void EnemyPlacementEditor::DrawDetailedEditor() {
#ifdef _DEBUG
	ImGui::Separator();
	myImGui::CenterText("Edit Selected Enemy");

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
		UpdatePreviewModelType(selectedPlacement);
		UpdatePreviewModelColor(selectedPlacement);
	}

	// パターン編集
	int editPattern = static_cast<int>(selectedPlacement.pattern);
	if (ImGui::Combo("Edit Pattern", &editPattern, kPatternDisplayNames, kPatternCount)) {
		selectedPlacement.pattern = static_cast<EnemyPattern>(editPattern);
	}

	// 待機時間編集
	ImGui::DragInt("Edit Wait Time", &selectedPlacement.waitTime, 1, 0, kMaxWaitTime);

	// スケール編集
	if (ImGui::DragFloat3("Model Scale", &selectedPlacement.modelScale.x, 0.1f, 0.1f, 10.0f)) {
		if (selectedPlacement.previewModel) {
			Vector3Transform transform{
				selectedPlacement.modelScale,
				{0.0f, 0.0f, 0.0f},
				selectedPlacement.position
			};
			selectedPlacement.previewModel->SetTransform(transform);
		}
	}
#endif
}