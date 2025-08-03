#include "RailCameraEditor.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "BaseSystem/Logger/Logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <format>
#include <chrono>

RailCameraEditor::RailCameraEditor() {
	memset(nameEditBuffer_, 0, sizeof(nameEditBuffer_));
}

void RailCameraEditor::Initialize(RailCamera* railCamera) {
	railCamera_ = railCamera;
	CreateDefaultPoints();
	LoadFromCSV(csvFilePath_);
	Logger::Log(Logger::GetStream(), "RailCameraEditor: Initialized\n");
}

void RailCameraEditor::Update() {
	if (!railCamera_) {
		return;
	}

	if (isDirty_) {
		ApplyToRailCamera();
		isDirty_ = false;
	}
}

void RailCameraEditor::ImGui() {
#ifdef _DEBUG
	if (!ImGui::Begin("Rail Camera Editor")) {
		ImGui::End();
		return;
	}

	ImGui::Text("=== Rail Camera Editor ===");
	ImGui::Separator();

	// メインコントロール
	ShowMainControls();
	ImGui::Separator();

	// 移動制御
	ShowMovementControls();
	ImGui::Separator();

	// デバッグ機能（GameSceneから移動）
	ShowDebugControls();
	ImGui::Separator();

	// 制御点リスト
	ShowControlPointsList();
	ImGui::Separator();

	// ファイル操作
	ShowFileOperations();
	ImGui::Separator();

	// クイックアクション
	ShowQuickActions();

	ImGui::End();
#endif
}

void RailCameraEditor::ShowMainControls() {
	ImGui::Text("Main Controls");

	if (!railCamera_) {
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No Rail Camera Connected!");
		return;
	}

	// カメラリセット
	if (ImGui::Button("Reset Camera Position")) {
		railCamera_->ResetPosition();
	}

	ImGui::Text("Points: %zu", points_.size());
}

void RailCameraEditor::ShowMovementControls() {
	if (ImGui::CollapsingHeader("Camera Movement")) {
		if (!railCamera_ || !railCamera_->GetMovement()) {
			return;
		}

		auto* movement = railCamera_->GetMovement();

		// 移動制御
		bool isMoving = movement->IsMoving();
		if (ImGui::Checkbox("Moving", &isMoving)) {
			if (isMoving) {
				movement->StartMovement();
			} else {
				movement->StopMovement();
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Start")) {
			movement->StartMovement();
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop")) {
			movement->StopMovement();
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset")) {
			movement->ResetPosition();
		}

		// ループ設定
		bool loopEnabled = movement->IsLoopEnabled();
		if (ImGui::Checkbox("Loop Movement", &loopEnabled)) {
			movement->SetLoopEnabled(loopEnabled);
		}

		// 速度設定
		float speed = movement->GetSpeed();
		if (ImGui::DragFloat("Speed", &speed, (1.0f / 6000.0f), (1.0f / 6000.0f), 0.001f, "%.9f")) {
			movement->SetSpeed(speed);
		}

		// 等間隔移動
		bool uniformSpeed = movement->IsUniformSpeedEnabled();
		if (ImGui::Checkbox("Uniform Speed", &uniformSpeed)) {
			movement->SetUniformSpeedEnabled(uniformSpeed);
			if (uniformSpeed && railCamera_->GetTrack()) {
				railCamera_->GetTrack()->BuildLengthTable();
			}
		}

		// 先読み距離
		float lookAhead = movement->GetLookAheadDistance();
		if (ImGui::DragFloat("Look Ahead Distance", &lookAhead, 0.001f)) {
			movement->SetLookAheadDistance(lookAhead);
		}

		// 進行度表示
		float progress = movement->GetProgress();
		if (ImGui::SliderFloat("Progress", &progress, 0.0f, 1.0f)) {
			movement->SetProgress(progress);
		}
	}
}

void RailCameraEditor::ShowDebugControls() {
	if (ImGui::CollapsingHeader("Debug Controls (from GameScene)")) {
		HandleFrameNavigation();
		ImGui::Separator();
		HandleProgressControl();
		ImGui::Separator();
		HandleVisualizationSettings();
		ImGui::Separator();
		HandleDebugInfo();
	}
}

void RailCameraEditor::HandleFrameNavigation() {
	ImGui::Text("=== Frame Navigation ===");

	if (!railCamera_ || !railCamera_->GetMovement()) {
		return;
	}

	auto* movement = railCamera_->GetMovement();
	int currentFrame = movement->GetCurrentFrameFromStart();
	int maxFrames = movement->GetMaxFrames();

	ImGui::Text("Current Frame: %d / %d", currentFrame, maxFrames);
	ImGui::Text("Progress: %.3f (%.1f%%)", movement->GetProgress(), movement->GetProgress() * 100.0f);

	// クイックジャンプボタン
	if (ImGui::Button("0%")) {
		movement->SetProgress(0.0f);
		movement->StopMovement();
	}
	ImGui::SameLine();
	if (ImGui::Button("25%")) {
		movement->SetProgress(0.25f);
		movement->StopMovement();
	}
	ImGui::SameLine();
	if (ImGui::Button("50%")) {
		movement->SetProgress(0.5f);
		movement->StopMovement();
	}
	ImGui::SameLine();
	if (ImGui::Button("75%")) {
		movement->SetProgress(0.75f);
		movement->StopMovement();
	}
	ImGui::SameLine();
	if (ImGui::Button("100%")) {
		movement->SetProgress(1.0f);
		movement->StopMovement();
	}

	// フレーム数入力
	ImGui::PushItemWidth(120);
	if (ImGui::InputInt("Target Frame", &debugFrameInput_)) {
		debugFrameInput_ = std::clamp(debugFrameInput_, 0, maxFrames);
	}
	ImGui::PopItemWidth();

	ImGui::SameLine();
	if (ImGui::Button("Jump")) {
		movement->SetProgressFromFrame(debugFrameInput_);
		movement->StopMovement();
	}

	ImGui::SameLine();
	if (ImGui::Button("Current->Input")) {
		debugFrameInput_ = currentFrame;
	}

	// フレームステップ操作
	if (ImGui::Button("-10")) {
		int newFrame = std::max(0, currentFrame - 10);
		movement->SetProgressFromFrame(newFrame);
		movement->StopMovement();
	}
	ImGui::SameLine();
	if (ImGui::Button("-1")) {
		int newFrame = std::max(0, currentFrame - 1);
		movement->SetProgressFromFrame(newFrame);
		movement->StopMovement();
	}
	ImGui::SameLine();
	if (ImGui::Button("+1")) {
		int newFrame = std::min(maxFrames, currentFrame + 1);
		movement->SetProgressFromFrame(newFrame);
		movement->StopMovement();
	}
	ImGui::SameLine();
	if (ImGui::Button("+10")) {
		int newFrame = std::min(maxFrames, currentFrame + 10);
		movement->SetProgressFromFrame(newFrame);
		movement->StopMovement();
	}

	// 時間換算表示
	ImGui::Text("=== Time (60FPS) ===");
	float seconds = currentFrame / 60.0f;
	int minutes = static_cast<int>(seconds) / 60;
	float remainingSeconds = seconds - (minutes * 60);
	ImGui::Text("Current: %d:%05.2f", minutes, remainingSeconds);

	float maxSeconds = maxFrames / 60.0f;
	int maxMinutes = static_cast<int>(maxSeconds) / 60;
	float maxRemainingSeconds = maxSeconds - (maxMinutes * 60);
	ImGui::Text("Total: %d:%05.2f", maxMinutes, maxRemainingSeconds);
}

void RailCameraEditor::HandleProgressControl() {
	ImGui::Text("=== Precise Progress Control ===");

	if (!railCamera_ || !railCamera_->GetMovement()) {
		return;
	}

	auto* movement = railCamera_->GetMovement();
	progressSlider_ = movement->GetProgress();

	if (ImGui::SliderFloat("Precise Progress", &progressSlider_, 0.0f, 1.0f, "%.4f")) {
		movement->SetProgress(progressSlider_);
		movement->StopMovement();
	}
}

void RailCameraEditor::HandleVisualizationSettings() {
	ImGui::Text("=== Visualization ===");

	if (!railCamera_ || !railCamera_->GetDebugger()) {
		return;
	}

	auto* debugger = railCamera_->GetDebugger();

	// 視錐台表示
	bool showFrustum = debugger->IsViewFrustumVisible();
	if (ImGui::Checkbox("Show View Frustum", &showFrustum)) {
		debugger->SetViewFrustumVisible(showFrustum);
	}

	if (showFrustum) {
		Vector4 frustumColor = debugger->GetViewFrustumColor();
		if (ImGui::ColorEdit4("Frustum Color", &frustumColor.x)) {
			debugger->SetViewFrustumColor(frustumColor);
		}

		float frustumDistance = debugger->GetViewFrustumDistance();
		if (ImGui::DragFloat("Frustum Distance", &frustumDistance, 1.0f, 5.0f, 200.0f)) {
			debugger->SetViewFrustumDistance(frustumDistance);
		}
	}

	// 軌道表示設定
	ImGui::Separator();
	bool showTrack = debugger->IsRailTrackVisible();
	if (ImGui::Checkbox("Show Rail Track", &showTrack)) {
		debugger->SetRailTrackVisible(showTrack);
	}

	if (showTrack) {
		Vector4 trackColor = debugger->GetRailTrackColor();
		if (ImGui::ColorEdit4("Track Color", &trackColor.x)) {
			debugger->SetRailTrackColor(trackColor);
			MarkDirty();
		}

		int segments = debugger->GetRailTrackSegments();
		if (ImGui::DragInt("Track Segments", &segments, 1, 10, 500)) {
			debugger->SetRailTrackSegments(segments);
			MarkDirty();
		}
	}

	// 制御点表示設定
	ImGui::Separator();
	bool showPoints = debugger->IsControlPointsVisible();
	if (ImGui::Checkbox("Show Control Points", &showPoints)) {
		debugger->SetControlPointsVisible(showPoints);
		MarkDirty();
	}

	if (showPoints) {
		Vector4 pointColor = debugger->GetControlPointColor();
		if (ImGui::ColorEdit4("Point Color", &pointColor.x)) {
			debugger->SetControlPointColor(pointColor);
			MarkDirty();
		}

		Vector4 selectedColor = debugger->GetSelectedPointColor();
		if (ImGui::ColorEdit4("Selected Color", &selectedColor.x)) {
			debugger->SetSelectedPointColor(selectedColor);
			MarkDirty();
		}

		float pointSize = debugger->GetControlPointSize();
		if (ImGui::DragFloat("Point Size", &pointSize, 0.01f, 0.1f, 2.0f)) {
			debugger->SetControlPointSize(pointSize);
			MarkDirty();
		}
	}
}

void RailCameraEditor::HandleDebugInfo() {
	ImGui::Text("=== Debug Information ===");

	if (!railCamera_) {
		return;
	}

	Vector3 cameraPos = railCamera_->GetPosition();
	Vector3 cameraRot = railCamera_->GetRotation();
	Vector3 forwardDir = railCamera_->GetForwardDirection();

	ImGui::Text("Position: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
	ImGui::Text("Rotation: (%.3f, %.3f, %.3f)", cameraRot.x, cameraRot.y, cameraRot.z);
	ImGui::Text("Forward: (%.3f, %.3f, %.3f)", forwardDir.x, forwardDir.y, forwardDir.z);

	if (railCamera_->GetTrack()) {
		ImGui::Text("Total Length: %.2f", railCamera_->GetTrack()->GetTotalLength());
	}
}

void RailCameraEditor::ShowControlPointsList() {
	ImGui::Text("Control Points:");

	if (ImGui::BeginChild("PointsList", ImVec2(0, 200), true)) {
		for (int i = 0; i < static_cast<int>(points_.size()); ++i) {
			ImGui::PushID(i);

			bool isSelected = (selectedPointIndex_ == i);
			if (isSelected) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.2f, 1.0f));
			}

			std::string headerLabel = std::format("[{}] {}", i, points_[i].GetName());
			if (ImGui::CollapsingHeader(headerLabel.c_str())) {
				// 名前編集
				if (editingNameIndex_ == i) {
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

				// 操作ボタン
				if (ImGui::Button("Select")) {
					selectedPointIndex_ = (selectedPointIndex_ == i) ? -1 : i;
					if (railCamera_ && railCamera_->GetDebugger()) {
						railCamera_->GetDebugger()->SetSelectedPointIndex(selectedPointIndex_);
						railCamera_->GenerateRailTrackLines();
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("Move Camera Here")) {
					MoveToPoint(i);
				}

				ImGui::SameLine();
				if (ImGui::Button("Delete")) {
					SafeRemovePoint(i);
					MarkDirty();
					ImGui::PopID();
					if (isSelected) {
						ImGui::PopStyleColor();
					}
					break;
				}
			}

			if (isSelected) {
				ImGui::PopStyleColor();
			}

			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	// 制御点追加
	ImGui::Separator();
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
}

void RailCameraEditor::ShowFileOperations() {
	ImGui::Text("File Operations:");

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
}

void RailCameraEditor::ShowQuickActions() {
	if (ImGui::CollapsingHeader("Quick Actions")) {
		if (ImGui::Button("Regenerate Track")) {
			if (railCamera_ && railCamera_->GetDebugger()) {
				railCamera_->GenerateRailTrackLines();
			}
		}

		if (selectedPointIndex_ >= 0 && IsValidIndex(selectedPointIndex_)) {
			ImGui::Separator();
			ImGui::Text("Selected Point [%d]:", selectedPointIndex_);

			RailPoint& selectedPoint = points_[selectedPointIndex_];

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
				if (railCamera_ && railCamera_->GetDebugger()) {
					railCamera_->GetDebugger()->SetSelectedPointIndex(-1);
					railCamera_->GenerateRailTrackLines();
				}
			}
		}
	}
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

		if (line.empty() || line[0] == '#' || line.find("//") == 0) {
			continue;
		}

		std::istringstream lineStream(line);
		std::string token;
		std::vector<std::string> tokens;

		while (std::getline(lineStream, token, ',')) {
			token.erase(0, token.find_first_not_of(" \t"));
			token.erase(token.find_last_not_of(" \t") + 1);
			tokens.push_back(token);
		}

		if (tokens.size() < 3) {
			continue;
		}

		try {
			Vector3 position;
			position.x = std::stof(tokens[0]);
			position.y = std::stof(tokens[1]);
			position.z = std::stof(tokens[2]);

			std::string name = (tokens.size() > 3) ? tokens[3] : "";
			points_.emplace_back(position, name);

		} catch (const std::exception&) {
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

	file << "x,y,z,name\n";

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
	if (LoadFromCSV(csvFilePath_)) {
		MarkDirty();
	}
}

void RailCameraEditor::SafeRemovePoint(int index) {
	if (!IsValidIndex(index)) {
		return;
	}

	if (selectedPointIndex_ == index) {
		selectedPointIndex_ = -1;
		if (railCamera_ && railCamera_->GetDebugger()) {
			railCamera_->GetDebugger()->SetSelectedPointIndex(-1);
		}
	} else if (selectedPointIndex_ > index) {
		selectedPointIndex_--;
	}

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