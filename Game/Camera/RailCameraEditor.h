#pragma once
#include "RailPoint.h"
#include "Camera/RailCamera.h"
#include <vector>
#include <string>
#include <memory>

/// <summary>
/// レールカメラの軌道エディタ
/// </summary>
class RailCameraEditor {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	RailCameraEditor();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~RailCameraEditor() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="railCamera">対象のレールカメラ</param>
	void Initialize(RailCamera* railCamera);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// CSVファイルから読み込み
	/// </summary>
	/// <param name="filename">ファイル名</param>
	/// <returns>成功したかどうか</returns>
	bool LoadFromCSV(const std::string& filename);

	/// <summary>
	/// CSVファイルに保存
	/// </summary>
	/// <param name="filename">ファイル名</param>
	/// <returns>成功したかどうか</returns>
	bool SaveToCSV(const std::string& filename);

	/// <summary>
	/// 選択したポイントにカメラを移動
	/// </summary>
	/// <param name="pointIndex">ポイントのインデックス</param>
	void MoveToPoint(int pointIndex);

	// Getter
	int GetSelectedPointIndex() const { return selectedPointIndex_; }

private:
	/// <summary>
	/// 制御点リスト
	/// </summary>
	std::vector<RailPoint> points_;

	/// <summary>
	/// 対象のレールカメラ
	/// </summary>
	RailCamera* railCamera_;

	/// <summary>
	/// 選択中の制御点インデックス
	/// </summary>
	int selectedPointIndex_;

	/// <summary>
	/// CSVファイルパス
	/// </summary>
	std::string csvFilePath_;

	/// <summary>
	/// 新しい点の座標
	/// </summary>
	Vector3 newPointPosition_;

	/// <summary>
	/// データが変更されたかどうかのフラグ
	/// </summary>
	bool isDirty_;

	/// <summary>
	/// RailCameraに制御点を適用
	/// </summary>
	void ApplyToRailCamera();

	/// <summary>
	/// 制御点をVector3のリストに変換
	/// </summary>
	/// <returns>Vector3のリスト</returns>
	std::vector<Vector3> ConvertToVector3List() const;

	/// <summary>
	/// デフォルトの制御点を生成
	/// </summary>
	void CreateDefaultPoints();

	/// <summary>
	/// 安全な制御点削除
	/// </summary>
	/// <param name="index">削除するインデックス</param>
	void SafeRemovePoint(int index);

	/// <summary>
	/// 制御点の範囲チェック
	/// </summary>
	/// <param name="index">チェックするインデックス</param>
	/// <returns>有効なインデックスかどうか</returns>
	bool IsValidIndex(int index) const;

	/// <summary>
	/// データ変更フラグを設定
	/// </summary>
	void MarkDirty();
};