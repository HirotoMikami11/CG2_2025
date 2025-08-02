#pragma once
#include <memory>
#include <vector>
#include <string>

#include "Objects/GameObject/GameObject.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/Light/Light.h"
#include "GameObjects/Enemy/BaseEnemy.h"
#include "MyMath/MyMath.h"

// 前方宣言
class EnemyPopCommand;
class CameraController;

/// <summary>
/// エディタで管理する敵配置データ
/// </summary>
struct EnemyPlacementData {
	Vector3 position;
	EnemyType enemyType;
	EnemyPattern pattern;
	int waitTime;           // この敵の前の待機時間
	bool isSelected;        // エディタで選択されているか
	std::unique_ptr<GameObject> previewModel; // プレビュー用モデル

	// デフォルトコンストラクタ
	EnemyPlacementData()
		: position{ 0.0f, 0.0f, 100.0f }
		, enemyType(EnemyType::Normal)
		, pattern(EnemyPattern::Straight)
		, waitTime(0)
		, isSelected(false)
		, previewModel(nullptr) {
	}

	// コピーコンストラクタを削除
	EnemyPlacementData(const EnemyPlacementData&) = delete;

	// コピー代入演算子を削除
	EnemyPlacementData& operator=(const EnemyPlacementData&) = delete;

	// ムーブコンストラクタ
	EnemyPlacementData(EnemyPlacementData&& other) noexcept
		: position(other.position)
		, enemyType(other.enemyType)
		, pattern(other.pattern)
		, waitTime(other.waitTime)
		, isSelected(other.isSelected)
		, previewModel(std::move(other.previewModel)) {
	}

	// ムーブ代入演算子
	EnemyPlacementData& operator=(EnemyPlacementData&& other) noexcept {
		if (this != &other) {
			position = other.position;
			enemyType = other.enemyType;
			pattern = other.pattern;
			waitTime = other.waitTime;
			isSelected = other.isSelected;
			previewModel = std::move(other.previewModel);
		}
		return *this;
	}
};

/// <summary>
/// 敵配置エディタクラス
/// </summary>
class EnemyPlacementEditor {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	EnemyPlacementEditor();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~EnemyPlacementEditor();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="cameraController">カメラコントローラーのポインタ</param>
	void Initialize(DirectXCommon* dxCommon, CameraController* cameraController = nullptr);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画（プレビューモデル）
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void Draw(const Light& directionalLight);

	/// <summary>
	/// ImGui描画
	/// </summary>
	void ImGui();

	/// <summary>
	/// CSV読み込み時に敵配置データを設定
	/// </summary>
	/// <param name="enemyPopCommand">敵生成コマンドのポインタ</param>
	void LoadFromEnemyPopCommand(EnemyPopCommand* enemyPopCommand);

	/// <summary>
	/// CSVに保存
	/// </summary>
	/// <param name="filePath">保存先ファイルパス</param>
	bool SaveToCSV(const std::string& filePath);

	/// <summary>
	/// CSVから読み込み
	/// </summary>
	/// <param name="filePath">読み込みファイルパス</param>
	bool LoadFromCSV(const std::string& filePath);

	// Getter
	bool IsEditorEnabled() const { return isEditorEnabled_; }
	bool IsPreviewVisible() const { return showPreviewModels_; }
	bool IsCSVEnemiesVisible() const { return showCSVEnemies_; }

private:
	// DirectXCommon参照
	DirectXCommon* directXCommon_;

	// CameraController参照
	CameraController* cameraController_;

	// エディタ有効フラグ
	bool isEditorEnabled_;

	// 表示設定
	bool showPreviewModels_;    // プレビューモデル表示
	bool showCSVEnemies_;       // CSV敵表示

	// 敵配置データ
	std::vector<EnemyPlacementData> enemyPlacements_;

	// 選択中のインデックス
	int selectedIndex_;

	// 新規追加用データ
	EnemyPlacementData newEnemyData_;

	// CSVファイルパス
	std::string csvFilePath_;

	/// <summary>
	/// プレビューモデルを作成
	/// </summary>
	/// <param name="placement">配置データ</param>
	/// <param name="isNewEnemy">新規追加用かどうか</param>
	void CreatePreviewModel(EnemyPlacementData& placement, bool isNewEnemy = false);

	/// <summary>
	/// プレビューモデルの色を更新
	/// </summary>
	/// <param name="placement">配置データ</param>
	void UpdatePreviewModelColor(EnemyPlacementData& placement);

	/// <summary>
	/// 敵タイプから文字列に変換
	/// </summary>
	/// <param name="type">敵タイプ</param>
	/// <returns>文字列</returns>
	std::string EnemyTypeToString(EnemyType type);

	/// <summary>
	/// 敵パターンから文字列に変換
	/// </summary>
	/// <param name="pattern">敵パターン</param>
	/// <returns>文字列</returns>
	std::string EnemyPatternToString(EnemyPattern pattern);

	/// <summary>
	/// 文字列から敵タイプに変換
	/// </summary>
	/// <param name="typeStr">文字列</param>
	/// <returns>敵タイプ</returns>
	EnemyType StringToEnemyType(const std::string& typeStr);

	/// <summary>
	/// 文字列から敵パターンに変換
	/// </summary>
	/// <param name="patternStr">文字列</param>
	/// <returns>敵パターン</returns>
	EnemyPattern StringToEnemyPattern(const std::string& patternStr);
};