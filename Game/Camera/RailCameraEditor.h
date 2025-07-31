#pragma once
#include "RailPoint.h"
#include "Camera/RailCamera.h"
#include <vector>
#include <string>
#include <memory>

/// <summary>
/// レールカメラの軌道エディタ（再設計版）
/// 安全性と使いやすさを重視した設計
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

	// Getter
	bool IsEnabled() const { return isEnabled_; }

	// Setter
	void SetEnabled(bool enabled) { isEnabled_ = enabled; }

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
	/// エディタが有効かどうか
	/// </summary>
	bool isEnabled_;

	/// <summary>
	/// 自動適用フラグ
	/// </summary>
	bool autoApply_;

	/// <summary>
	/// CSVファイルパス
	/// </summary>
	std::string csvFilePath_;

	/// <summary>
	/// 新しい点の座標
	/// </summary>
	Vector3 newPointPosition_;

	/// <summary>
	/// 最後に適用した時刻（ログ制御用）
	/// </summary>
	float lastApplyTime_;

	/// <summary>
	/// ログ間隔（秒）
	/// </summary>
	static constexpr float kLogInterval_ = 2.0f;

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
	/// 時刻取得（ログ制御用）
	/// </summary>
	/// <returns>現在時刻（秒）</returns>
	float GetCurrentTimes() const;
};