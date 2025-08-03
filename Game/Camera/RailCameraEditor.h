#pragma once
#include "RailPoint.h"
#include "Camera/RailCamera.h"
#include <vector>
#include <string>
#include <memory>

/// <summary>
/// レールカメラの統合エディタ
/// すべてのImGui操作を統一して管理
/// </summary>
class RailCameraEditor {
public:
	RailCameraEditor();
	~RailCameraEditor() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(RailCamera* railCamera);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 統合ImGui表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// CSVファイルから読み込み
	/// </summary>
	bool LoadFromCSV(const std::string& filename);

	/// <summary>
	/// CSVファイルに保存
	/// </summary>
	bool SaveToCSV(const std::string& filename);

private:
	// サブセクションの表示
	void ShowMainControls();
	void ShowMovementControls();
	void ShowDebugControls();
	void ShowControlPointsList();
	void ShowFileOperations();
	void ShowQuickActions();

	// 操作関数
	void MoveToPoint(int pointIndex);
	void ApplyToRailCamera();
	std::vector<Vector3> ConvertToVector3List() const;
	void CreateDefaultPoints();
	void SafeRemovePoint(int index);
	bool IsValidIndex(int index) const;
	void MarkDirty();

	// 名前編集
	void StartNameEdit(int pointIndex);
	void ConfirmNameEdit();
	void CancelNameEdit();

	// デバッグ操作（GameSceneから移動）
	void HandleFrameNavigation();
	void HandleProgressControl();
	void HandleVisualizationSettings();
	void HandleDebugInfo();

	// メンバ変数
	std::vector<RailPoint> points_;
	RailCamera* railCamera_ = nullptr;

	int selectedPointIndex_ = -1;
	std::string csvFilePath_ = "resources/CSV_Data/RailCameraPoints/Stage0";
	Vector3 newPointPosition_ = { 0.0f, 0.0f, 0.0f };
	bool isDirty_ = false;

	// 名前編集
	char nameEditBuffer_[256];
	int editingNameIndex_ = -1;

	// デバッグ用（GameSceneから移動）
	int debugFrameInput_ = 0;
	float progressSlider_ = 0.0f;
};