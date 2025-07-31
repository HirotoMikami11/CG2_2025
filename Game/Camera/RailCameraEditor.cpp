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
	, isEnabled_(true)
	, autoApply_(true)
	, csvFilePath_("resources/CSV_Data/RailCamera/rail_points.csv")
	, newPointPosition_{ 0.0f, 0.0f, 0.0f }
	, lastApplyTime_(0.0f)
{
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
	if (!isEnabled_ || !railCamera_) {
		return;
	}

	// 自動適用が有効で、一定時間経過した場合のみ適用
	if (autoApply_) {
		float currentTime = GetCurrentTime();
		if (currentTime - lastApplyTime_ > 0.1f) { // 0.1秒間隔で適用
			ApplyToRailCamera();
			lastApplyTime_ = currentTime;
		}
	}
}

void RailCameraEditor::ImGui() {
#ifdef _DEBUG
	if (!isEnabled_) {
		return;
	}

	ImGui::Begin("Rail Camera Editor");

	// === メインコントロール ===
	ImGui::Text("Rail Camera Editor");

	if (ImGui::Button("Apply to Camera")) {
		ApplyToRailCamera();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset Camera Position")) {
		if (railCamera_) {
			railCamera_->ResetPosition();
		}
	}

	ImGui::Text("Points: %zu", points_.size());

	ImGui::Separator();

	// === 制御点リスト ===
	ImGui::Text("Control Points:");

	if (ImGui::BeginChild("PointsList", ImVec2(0, 200), true)) {
		for (int i = 0; i < static_cast<int>(points_.size()); ++i) {
			ImGui::PushID(i);

			// 選択状態の表示
			bool isSelected = (selectedPointIndex_ == i);
			if (isSelected) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
			}

			// 制御点の情報表示と編集
			std::string headerLabel = std::format("[{}] {}", i, points_[i].GetName());
			if (ImGui::CollapsingHeader(headerLabel.c_str())) {

				// 名前編集
				char nameBuffer[256];
				strcpy_s(nameBuffer, sizeof(nameBuffer), points_[i].GetName().c_str());
				if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
					points_[i].SetName(nameBuffer);
				}

				// 座標編集
				Vector3 pos = points_[i].GetPosition();
				if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
					points_[i].SetPosition(pos);
				}

				// 選択ボタン
				if (ImGui::Button("Select")) {
					selectedPointIndex_ = (selectedPointIndex_ == i) ? -1 : i;
				}

				ImGui::SameLine();
				// 削除ボタン
				if (ImGui::Button("Delete")) {
					SafeRemovePoint(i);
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
	}
	ImGui::SameLine();
	if (ImGui::Button("Add at Camera")) {
		if (railCamera_) {
			points_.emplace_back(railCamera_->GetPosition());
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Add at Origin")) {
		points_.emplace_back(Vector3{ 0.0f, 0.0f, 0.0f });
	}

	if (ImGui::Button("Clear All Points")) {
		points_.clear();
		selectedPointIndex_ = -1;
	}

	ImGui::Separator();

	// === ファイル操作 ===
	ImGui::Text("File Operations:");

	// ファイルパス入力
	char pathBuffer[512];
	strcpy_s(pathBuffer, sizeof(pathBuffer), csvFilePath_.c_str());
	if (ImGui::InputText("CSV File Path", pathBuffer, sizeof(pathBuffer))) {
		csvFilePath_ = pathBuffer;
	}

	if (ImGui::Button("Load CSV")) {
		LoadFromCSV(csvFilePath_);
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

		std::string filename = std::format("resources/CSV_Data/RailCamera/rail_points_{}.csv", timestamp);
		SaveToCSV(filename);
	}

	ImGui::Separator();

	// === 設定 ===
	ImGui::Text("Settings:");

	ImGui::Checkbox("Auto Apply", &autoApply_);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Automatically apply changes to rail camera");
	}

	ImGui::Checkbox("Editor Enabled", &isEnabled_);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Enable/disable the editor");
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
	// デフォルトの制御点
	std::vector<Vector3> defaultPoints = {
		{0,  0,   0 },
		{10, 10,  0 },
		{10, 15,  0 },
		{20, 15,  0 },
		{20, 0,   0 },
		{20, 0,   10},
		{30, -10, 5 },
		{20, -10, 0 },
		{20, -15, 5 },
		{10, -15, 0 },
		{10, -10, 0 },
		{0,  0,   0 },
	};

	for (size_t i = 0; i < defaultPoints.size(); ++i) {
		points_.emplace_back(defaultPoints[i], std::format("DefaultPoint_{}", i));
	}
}

void RailCameraEditor::SafeRemovePoint(int index) {
	if (!IsValidIndex(index)) {
		return;
	}

	// 選択中の点を削除する場合は選択を解除
	if (selectedPointIndex_ == index) {
		selectedPointIndex_ = -1;
	} else if (selectedPointIndex_ > index) {
		selectedPointIndex_--;
	}

	points_.erase(points_.begin() + index);
}

bool RailCameraEditor::IsValidIndex(int index) const {
	return index >= 0 && index < static_cast<int>(points_.size());
}

float RailCameraEditor::GetCurrentTimes() const {
	static auto startTime = std::chrono::steady_clock::now();
	auto currentTime = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
	return static_cast<float>(duration.count()) / 1000.0f;
}