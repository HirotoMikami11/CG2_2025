#include "RailCameraEditor.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "BaseSystem/Logger/Logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <format>
#include <chrono>

RailCameraEditor::RailCameraEditor()
	: railCamera_(nullptr)
	, selectedPointIndex_(-1)
	, csvFilePath_("resources/CSV_Data/RailCameraPoints/Stage0")
	, newPointPosition_{ 0.0f, 0.0f, 0.0f }
	, isDirty_(false)
	, editingNameIndex_(-1)
{
	// 名前編集バッファを初期化
	memset(nameEditBuffer_, 0, sizeof(nameEditBuffer_));
}

void RailCameraEditor::Initialize(RailCamera* railCamera) {
	railCamera_ = railCamera;

	// デフォルトの制御点を作成
	CreateDefaultPoints();

	// CSVファイルが存在する場合は読み込む
	LoadFromCSV(csvFilePath_);

	Logger::Log(Logger::GetStream(), "RailCameraEditor: Initialized\n");
}

void RailCameraEditor::Update() {
	if (!railCamera_) {
		return;
	}

	// データが変更された場合のみ適用
	if (isDirty_) {
		ApplyToRailCamera();
		isDirty_ = false;
	}
}

void RailCameraEditor::ImGui() {
#ifdef _DEBUG
	ImGui::Begin("Rail Camera Editor");

	// === メインコントロール ===
	ImGui::Text("Rail Camera Editor");

	if (ImGui::Button("Reset Camera Position")) {
		if (railCamera_) {
			railCamera_->ResetPosition();
		}
	}

	ImGui::Text("Points: %zu", points_.size());

	ImGui::Separator();

	// === カメラ制御設定 ===
	if (railCamera_) {
		ImGui::Text("Camera Controls:");

		// 移動制御
		bool isMoving = railCamera_->IsMoving();
		if (ImGui::Checkbox("Moving", &isMoving)) {
			if (isMoving) {
				railCamera_->StartMovement();
			} else {
				railCamera_->StopMovement();
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Start")) {
			railCamera_->StartMovement();
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop")) {
			railCamera_->StopMovement();
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset")) {
			railCamera_->ResetPosition();
		}

		// ループ設定
		bool loopEnabled = railCamera_->IsLoopEnabled();
		if (ImGui::Checkbox("Loop Movement", &loopEnabled)) {
			railCamera_->SetLoopEnabled(loopEnabled);
		}

		// 速度設定
		float speed = railCamera_->GetSpeed();
		if (ImGui::DragFloat("Speed", &speed, (1.0f / 6000.0f), (1.0f / 6000.0f), 0.00001f)) {
			railCamera_->SetSpeed(speed);
		}
		ImGui::Text("Speed: %.9f", speed);

		// 進行度表示と設定
		float progress = railCamera_->GetProgress();
		if (ImGui::SliderFloat("Progress", &progress, 0.0f, 1.0f)) {
			railCamera_->SetProgress(progress);
		}

		ImGui::Separator();
	}

	// === 制御点リスト ===
	ImGui::Text("Control Points:");

	if (ImGui::BeginChild("PointsList", ImVec2(0, 200), true)) {
		for (int i = 0; i < static_cast<int>(points_.size()); ++i) {
			ImGui::PushID(i);

			// 選択状態の表示（色を変更）
			bool isSelected = (selectedPointIndex_ == i);
			if (isSelected) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.2f, 1.0f)); // 黄色
			}

			// 制御点の情報表示と編集
			std::string headerLabel = std::format("[{}] {}", i, points_[i].GetName());
			if (ImGui::CollapsingHeader(headerLabel.c_str())) {

				// 名前編集
				if (editingNameIndex_ == i) {
					// 編集中の場合
					ImGui::InputText("##EditName", nameEditBuffer_, sizeof(nameEditBuffer_));

					ImGui::SameLine();
					if (ImGui::Button("OK") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
						ConfirmNameEdit();
					}

					ImGui::SameLine();
					if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
						CancelNameEdit();
					}
				} else {
					// 通常表示
					ImGui::Text("Name: %s", points_[i].GetName().c_str());
					ImGui::SameLine();
					if (ImGui::Button("Edit Name")) {
						StartNameEdit(i);
					}
				}

				// 座標編集
				Vector3 pos = points_[i].GetPosition();
				if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
					points_[i].SetPosition(pos);
					MarkDirty();
				}

				// 選択ボタン
				if (ImGui::Button("Select")) {
					selectedPointIndex_ = (selectedPointIndex_ == i) ? -1 : i;
					// 制御点の色を更新するためにRailCameraを更新
					if (railCamera_) {
						railCamera_->SetSelectedPointIndex(selectedPointIndex_);
						railCamera_->GenerateRailTrackLines();
					}
				}

				ImGui::SameLine();
				// カメラ移動ボタン
				if (ImGui::Button("Move Camera Here")) {
					MoveToPoint(i);
				}

				ImGui::SameLine();
				// 削除ボタン
				if (ImGui::Button("Delete")) {
					SafeRemovePoint(i);
					MarkDirty();
					ImGui::PopID();
					if (isSelected) {
						ImGui::PopStyleColor();
					}
					break; // ループを抜ける
				}
			}

			if (isSelected) {
				ImGui::PopStyleColor();
			}

			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	ImGui::Separator();

	// === 制御点追加 ===
	ImGui::Text("Add New Point:");
	ImGui::DragFloat3("Position", &newPointPosition_.x, 0.1f);

	if (ImGui::Button("Add Point")) {
		points_.emplace_back(newPointPosition_);
		MarkDirty();
	}
	ImGui::SameLine();
	if (ImGui::Button("Add at Camera")) {
		if (railCamera_) {
			points_.emplace_back(railCamera_->GetPosition());
			MarkDirty();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Add at Origin")) {
		points_.emplace_back(Vector3{ 0.0f, 0.0f, 0.0f });
		MarkDirty();
	}

	ImGui::Separator();

	// === 選択されたポイントの詳細編集 ===
	if (selectedPointIndex_ >= 0 && IsValidIndex(selectedPointIndex_)) {
		ImGui::Text("Selected Point [%d]:", selectedPointIndex_);

		// 選択されたポイントの背景色を変更
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.3f, 0.0f, 0.3f)); // 薄い黄色背景

		RailPoint& selectedPoint = points_[selectedPointIndex_];

		// 名前編集（選択されたポイント用）
		if (editingNameIndex_ == selectedPointIndex_) {
			ImGui::InputText("Selected Name", nameEditBuffer_, sizeof(nameEditBuffer_));

			ImGui::SameLine();
			if (ImGui::Button("OK##Selected") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
				ConfirmNameEdit();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel##Selected") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				CancelNameEdit();
			}
		} else {
			ImGui::Text("Name: %s", selectedPoint.GetName().c_str());
			ImGui::SameLine();
			if (ImGui::Button("Edit Name##Selected")) {
				StartNameEdit(selectedPointIndex_);
			}
		}

		// 座標編集
		Vector3 pos = selectedPoint.GetPosition();
		if (ImGui::DragFloat3("Selected Position", &pos.x, 0.1f)) {
			selectedPoint.SetPosition(pos);
			MarkDirty();
		}

		ImGui::PopStyleColor(); // 背景色をリセット

		// 操作ボタン
		if (ImGui::Button("Move Camera to Selected")) {
			MoveToPoint(selectedPointIndex_);
		}
		ImGui::SameLine();
		if (ImGui::Button("Update from Camera")) {
			if (railCamera_) {
				selectedPoint.SetPosition(railCamera_->GetPosition());
				MarkDirty();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Deselect")) {
			selectedPointIndex_ = -1;
			if (railCamera_) {
				railCamera_->SetSelectedPointIndex(-1);
				railCamera_->GenerateRailTrackLines();
			}
		}

		ImGui::Separator();
	}

	// === ファイル操作 ===
	ImGui::Text("File Operations:");

	// ファイルパス入力
	char pathBuffer[512];
	strcpy_s(pathBuffer, sizeof(pathBuffer), csvFilePath_.c_str());
	if (ImGui::InputText("CSV File Path", pathBuffer, sizeof(pathBuffer))) {
		csvFilePath_ = pathBuffer;
	}

	if (ImGui::Button("Load CSV")) {
		if (LoadFromCSV(csvFilePath_)) {
			MarkDirty();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Save CSV")) {
		SaveToCSV(csvFilePath_);
	}
	ImGui::SameLine();
	if (ImGui::Button("Save As...")) {
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
			nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
		std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };
		std::string timestamp = std::format("{:%Y%m%d_%H%M%S}", localTime);

		std::string filename = std::format("resources/CSV_Data/RailCameraPoints/railCameraPoints_{}.csv", timestamp);
		SaveToCSV(filename);
	}

	ImGui::End();
#endif
}

bool RailCameraEditor::LoadFromCSV(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		Logger::Log(Logger::GetStream(),
			std::format("RailCameraEditor: Failed to open file: {}\n", filename));
		return false;
	}

	points_.clear();
	selectedPointIndex_ = -1;
	editingNameIndex_ = -1;

	std::string line;
	int lineNumber = 0;

	// ヘッダー行をスキップ
	if (std::getline(file, line)) {
		lineNumber++;
	}

	while (std::getline(file, line)) {
		lineNumber++;

		// コメント行や空行をスキップ
		if (line.empty() || line[0] == '#' || line.find("//") == 0) {
			continue;
		}

		std::istringstream lineStream(line);
		std::string token;
		std::vector<std::string> tokens;

		// カンマ区切りでトークンを分割
		while (std::getline(lineStream, token, ',')) {
			// 前後の空白を削除
			token.erase(0, token.find_first_not_of(" \t"));
			token.erase(token.find_last_not_of(" \t") + 1);
			tokens.push_back(token);
		}

		// 最低3つの値（x, y, z）が必要
		if (tokens.size() < 3) {
			continue;
		}

		try {
			Vector3 position;
			position.x = std::stof(tokens[0]);
			position.y = std::stof(tokens[1]);
			position.z = std::stof(tokens[2]);

			// 名前が指定されている場合は使用、なければデフォルト
			std::string name = (tokens.size() > 3) ? tokens[3] : "";

			points_.emplace_back(position, name);

		} catch (const std::exception&) {
			// エラーは無視して次の行へ
			continue;
		}
	}

	file.close();

	Logger::Log(Logger::GetStream(),
		std::format("RailCameraEditor: Loaded {} points from {}\n",
			points_.size(), filename));

	return true;
}

bool RailCameraEditor::SaveToCSV(const std::string& filename) {
	std::ofstream file(filename);
	if (!file.is_open()) {
		Logger::Log(Logger::GetStream(),
			std::format("RailCameraEditor: Failed to create file: {}\n", filename));
		return false;
	}

	// ヘッダー行
	file << "x,y,z,name\n";

	// 制御点データ
	for (const auto& point : points_) {
		const Vector3& pos = point.GetPosition();
		file << std::format("{:.6f},{:.6f},{:.6f},{}\n",
			pos.x, pos.y, pos.z, point.GetName());
	}

	file.close();

	Logger::Log(Logger::GetStream(),
		std::format("RailCameraEditor: Saved {} points to {}\n",
			points_.size(), filename));

	return true;
}

void RailCameraEditor::MoveToPoint(int pointIndex) {
	if (!railCamera_ || !IsValidIndex(pointIndex)) {
		return;
	}

	const Vector3& targetPosition = points_[pointIndex].GetPosition();
	railCamera_->SetPosition(targetPosition);

	// カメラの移動を停止して、現在位置から操作できるようにする
	railCamera_->StopMovement();
}

void RailCameraEditor::ApplyToRailCamera() {
	if (!railCamera_ || points_.size() < 4) {
		return;
	}

	std::vector<Vector3> controlPoints = ConvertToVector3List();
	railCamera_->SetControlPoints(controlPoints);
}

std::vector<Vector3> RailCameraEditor::ConvertToVector3List() const {
	std::vector<Vector3> result;
	result.reserve(points_.size());

	for (const auto& point : points_) {
		result.push_back(point.GetPosition());
	}

	return result;
}

void RailCameraEditor::CreateDefaultPoints() {
	// デフォルトは最初のパスから持ってくる
	if (LoadFromCSV(csvFilePath_)) {
		MarkDirty();
	}
}

void RailCameraEditor::SafeRemovePoint(int index) {
	if (!IsValidIndex(index)) {
		return;
	}

	// 選択中の点を削除する場合は選択を解除
	if (selectedPointIndex_ == index) {
		selectedPointIndex_ = -1;
		if (railCamera_) {
			railCamera_->SetSelectedPointIndex(-1);
		}
	} else if (selectedPointIndex_ > index) {
		selectedPointIndex_--;
	}

	// 編集中の点を削除する場合は編集を終了
	if (editingNameIndex_ == index) {
		editingNameIndex_ = -1;
	} else if (editingNameIndex_ > index) {
		editingNameIndex_--;
	}

	points_.erase(points_.begin() + index);
}

bool RailCameraEditor::IsValidIndex(int index) const {
	return index >= 0 && index < static_cast<int>(points_.size());
}

void RailCameraEditor::MarkDirty() {
	isDirty_ = true;
}

void RailCameraEditor::StartNameEdit(int pointIndex) {
	if (!IsValidIndex(pointIndex)) {
		return;
	}

	editingNameIndex_ = pointIndex;

	// 現在の名前をバッファにコピー
	const std::string& currentName = points_[pointIndex].GetName();
	strcpy_s(nameEditBuffer_, sizeof(nameEditBuffer_), currentName.c_str());
}

void RailCameraEditor::ConfirmNameEdit() {
	if (editingNameIndex_ >= 0 && IsValidIndex(editingNameIndex_)) {
		points_[editingNameIndex_].SetName(nameEditBuffer_);
		MarkDirty();
	}

	editingNameIndex_ = -1;
	memset(nameEditBuffer_, 0, sizeof(nameEditBuffer_));
}

void RailCameraEditor::CancelNameEdit() {
	editingNameIndex_ = -1;
	memset(nameEditBuffer_, 0, sizeof(nameEditBuffer_));
}